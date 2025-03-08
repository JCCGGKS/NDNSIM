/NFD/daemon/fw/forwarder.cpp

# NDN路由转发流程分析
## 收到兴趣包，upstream
### onInterestLoop
```cpp
void Forwarder::onIncomingInterest(Face& inFace, const Interest& interest){
  // receive Interest
  // 记录日志
  NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                " interest=" << interest.getName());

  // interest的顶层基类是TagHost，继承基类的setTag方法
  // 记录兴趣包传入FaceId    
  interest.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));

  // 收到的兴趣包个数增加
  ++m_counters.nInInterests;


  // localhost scope control
  /**
  * FACE_SCOPE_NON_LOCAL定义在/ndn-cxx/encoding/nfd-constants.hpp
  * FACE_SCOPE_NON_LOCAL = 0, ///< face is non-local

  * scope_prefix::LOCALHOST声明在/NFD/core/scope-prefix.hpp
  * 定义在/NFD/core/scope-prefix.cpp
  * const Name LOCALHOST("ndn:/localhost");
  */
  bool isViolatingLocalhost = inFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(interest.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                  " interest=" << interest.getName() << " violates /localhost");
    // (drop)
    return;
  }


  // detect duplicate Nonce with Dead Nonce List
  bool hasDuplicateNonceInDnl = m_deadNonceList.has(interest.getName(), interest.getNonce());
  // 收到重复兴趣包，进入兴趣重复管道
  if (hasDuplicateNonceInDnl) {
    // goto Interest loop pipeline
    this->onInterestLoop(inFace, interest);
    return;
  }

  // strip forwarding hint if Interest has reached producer region
  // 如果兴趣包已到达生产者，则直接转发提示
  if (!interest.getForwardingHint().empty() &&
      m_networkRegionTable.isInProducerRegion(interest.getForwardingHint())) {
    NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                  " interest=" << interest.getName() << " reaching-producer-region");

    // 传入的interest是一个常量对象，而setForwardingHint是一个非常量函数，所以需要先用const_cast去掉常量属性
    const_cast<Interest&>(interest).setForwardingHint({});
  }

/******************************************************************************************/
  // PIT insert
  // 调用nfd::pit::findOrInsert(interest);返回std::pair<shared_ptr<Entry>,bool>,获取第一个返回值
  // 获取的pitEntry有可能是已经存在的，也有可能是新插入的
  shared_ptr<pit::Entry> pitEntry = m_pit.insert(interest).first;


  // detect duplicate Nonce in PIT entry
  int dnw = fw::findDuplicateNonce(*pitEntry, interest.getNonce(), inFace);

  bool hasDuplicateNonceInPit = dnw != fw::DUPLICATE_NONCE_NONE;

  if (inFace.getLinkType() == ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    // for p2p face: duplicate Nonce from same incoming face is not loop
    // 来自同一输入face的重复Nonce不是循环
    hasDuplicateNonceInPit = hasDuplicateNonceInPit && !(dnw & fw::DUPLICATE_NONCE_IN_SAME);
  }

  if (hasDuplicateNonceInPit) {
    // goto Interest loop pipeline
    this->onInterestLoop(inFace, interest);
    return;
  }

  // 查询缓存
  // is pending?
  // hashInRecords定义在/NFD/daemon/table/pit-entry.hpp
  // There is no in-record. This implies the entry is new or has been satisfied or Nacked.
  // 如果没有in-record，暗示entry是新的或者已经被满足或者Nacked
  if (!pitEntry->hasInRecords()) {
    if (m_csFromNdnSim == nullptr) {
    //在新版CS中查询缓存
      m_cs.find(interest,
                bind(&Forwarder::onContentStoreHit, this, std::ref(inFace), pitEntry, _1, _2),
                bind(&Forwarder::onContentStoreMiss, this, std::ref(inFace), pitEntry, _1));
    }
    else {
    //在旧版CS中查询缓存
      shared_ptr<Data> match = m_csFromNdnSim->Lookup(interest.shared_from_this());
      if (match != nullptr) {
        // 缓存命中
        this->onContentStoreHit(inFace, pitEntry, interest, *match);
      }
      else {
        // 缓存未命中
        this->onContentStoreMiss(inFace, pitEntry, interest);
      }
    }
  }
  else {
    // PIT entry是新的之前没有请求过，缓存中没有存储该内容相当于缓存失效
    this->onContentStoreMiss(inFace, pitEntry, interest);
  }
}
/******************************************************************************************/

```
### onInterestLoop
```cpp
void Forwarder::onInterestLoop(Face& inFace, const Interest& interest){
  // if multi-access or ad hoc face, drop
  if (inFace.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onInterestLoop face=" << inFace.getId() <<
                  " interest=" << interest.getName() <<
                  " drop");
    return;
  }

  NFD_LOG_DEBUG("onInterestLoop face=" << inFace.getId() <<
                " interest=" << interest.getName() <<
                " send-Nack-duplicate");

  // send Nack with reason=DUPLICATE
  // note: Don't enter outgoing Nack pipeline because it needs an in-record.
  lp::Nack nack(interest);
  nack.setReason(lp::NackReason::DUPLICATE);
  inFace.sendNack(nack);
}
```
### onContentStoreHit
```cpp
void Forwarder::onContentStoreHit(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                             const Interest& interest, const Data& data){
  // 记录日志
  NFD_LOG_DEBUG("onContentStoreHit interest=" << interest.getName());
  // 缓存命中数递增
  ++m_counters.nCsHits;


  data.setTag(make_shared<lp::IncomingFaceIdTag>(face::FACEID_CONTENT_STORE));
  // XXX should we lookup PIT for other Interests that also match csMatch?
  // 查找PIT满足待满足的兴趣包

  pitEntry->isSatisfied = true;
  pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

  // set PIT expiry timer to now
  // 该PIT entry已经被满足，设置过期时间为现在
  this->setExpiryTimer(pitEntry, 0_ms);


  beforeSatisfyInterest(*pitEntry, *m_csFace, data);
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, *m_csFace, data); });

  // dispatch to strategy: after Content Store hit
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterContentStoreHit(pitEntry, inFace, data); });
}
```

### onContentStoreMiss

```cpp
void Forwarder::onContentStoreMiss(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                              const Interest& interest){
  // 记录日志  
  NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName());
  // 缓存未命中次数递增
  ++m_counters.nCsMisses;

  // insert in-record
  // 聚合对同一个兴趣包请求的InComingFace标识
  pitEntry->insertOrUpdateInRecord(const_cast<Face&>(inFace), interest);

  // set PIT expiry timer to the time that the last PIT in-record expires
  // 设置PIT过期时间为最后一个PIT in-records的过期时间
  auto lastExpiring = std::max_element(pitEntry->in_begin(), pitEntry->in_end(), &compare_InRecord_expiry);
  // 计算过期时间距离现在的时间间隙
  auto lastExpiryFromNow = lastExpiring->getExpiry() - time::steady_clock::now();

  this->setExpiryTimer(pitEntry, time::duration_cast<time::milliseconds>(lastExpiryFromNow));

  // has NextHopFaceId?
  shared_ptr<lp::NextHopFaceIdTag> nextHopTag = interest.getTag<lp::NextHopFaceIdTag>();
  if (nextHopTag != nullptr) {
    // chosen NextHop face exists?
    Face* nextHopFace = m_faceTable.get(*nextHopTag);

    if (nextHopFace != nullptr) {
      NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName() << " nexthop-faceid=" << nextHopFace->getId());

      // go to outgoing Interest pipeline
      // scope control is unnecessary, because privileged app explicitly wants to forward
      // 从nextHopFace转发兴趣包
      this->onOutgoingInterest(pitEntry, *nextHopFace, interest);
    }
    return;
  }

  // dispatch to strategy: after incoming Interest
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterReceiveInterest(inFace, interest, pitEntry); });
}
```
## 收到数据包，downstream
```cpp
//收到DataPacket
void Forwarder::onIncomingData(Face& inFace, const Data& data){

  // receive Data
  // 记录日志
  NFD_LOG_DEBUG("onIncomingData face=" << inFace.getId() << " data=" << data.getName());

  // Data的顶层基类是TagHost，继承基类的setTag方法
  data.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));
  // 递增收到的数据包个数
  ++m_counters.nInData;

  // /localhost scope control
  bool isViolatingLocalhost = inFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onIncomingData face=" << inFace.getId() <<
                  " data=" << data.getName() << " violates /localhost");
    // (drop)
    return;
  }

/*********************************************************************************/
  // PIT match
  // 找到所有请求该数据包的PIT entry
  pit::DataMatchResult pitMatches = m_pit.findAllDataMatches(data);

  // 没有兴趣包请求该数据包
  if (pitMatches.size() == 0) {
    // goto Data unsolicited pipeline
    //在PIT表中找不到对该数据包的请求条目，将其送入未请求通道
    this->onDataUnsolicited(inFace, data);
    return;
  }

  // 不带Tag的DataPacket的副本
  shared_ptr<Data> dataCopyWithoutTag = make_shared<Data>(data);
  // 去除副本的HopCountTag
  // HopCountTag记录的是包经过的路由节点个数，副本被缓存之后不会被转发故去除该Tag
  dataCopyWithoutTag->removeTag<lp::HopCountTag>();

  /*********************************CS Insert：LCE***************************/
  // CS insert
  //按照某种策略缓存该数据包
  //m_cs：2版本实现的ContentStore
  //m_csFromNdnSim：1版本实现的ContentStore

  //默认的LCE策略
  if (m_csFromNdnSim == nullptr)
    m_cs.insert(*dataCopyWithoutTag);
  else
    m_csFromNdnSim->Add(dataCopyWithoutTag);
  /*********************************CS Insert：LCE***************************/


  // when only one PIT entry is matched, trigger strategy: after receive Data
  // 当只有一个PIT表项匹配时，触发策略:接收到Data后触发
  if (pitMatches.size() == 1) {
    auto& pitEntry = pitMatches.front();

    NFD_LOG_DEBUG("onIncomingData matching=" << pitEntry->getName());

    // set PIT expiry timer to now
    this->setExpiryTimer(pitEntry, 0_ms);

    beforeSatisfyInterest(*pitEntry, inFace, data);
    // trigger strategy: after receive Data
    this->dispatchToStrategy(*pitEntry,
      [&] (fw::Strategy& strategy) { strategy.afterReceiveData(pitEntry, inFace, data); });

    // mark PIT satisfied
    pitEntry->isSatisfied = true;
    pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

    // Dead Nonce List insert if necessary (for out-record of inFace)
    this->insertDeadNonceList(*pitEntry, &inFace);

    // delete PIT entry's out-record
    pitEntry->deleteOutRecord(inFace);
  }
  // when more than one PIT entry is matched, trigger strategy: before satisfy Interest,
  // and send Data to all matched out faces
  // 当多个PIT条目匹配时，触发策略:在满足Interest之前，并向所有匹配的out faces发送数据
  else {
  /*********************************send data to downstream********************/

    // 记录
    std::set<Face*> pendingDownstreams;
    // 获取当前的时间
    auto now = time::steady_clock::now();

    // 遍历所有请求该数据包的Pit entry,获取InFaceId。
    // 之后将Data从所有InFaceId传出，满足请求该数据包待满足的兴趣包
    for (const shared_ptr<pit::Entry>& pitEntry : pitMatches) {
      // 记录日志
      NFD_LOG_DEBUG("onIncomingData matching=" << pitEntry->getName());

      // remember pending downstreams
      // 遍历并存储所有InFaceId
      for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {

      // 兴趣包必须在生命周期(InterestLifetime)过期之前，收到数据包才算有效被响应
        if (inRecord.getExpiry() > now) {
          pendingDownstreams.insert(&inRecord.getFace());
        }
      }

      // set PIT expiry timer to now
      this->setExpiryTimer(pitEntry, 0_ms);

      // invoke PIT satisfy callback
      beforeSatisfyInterest(*pitEntry, inFace, data);
      this->dispatchToStrategy(*pitEntry,
        [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, inFace, data); });

      // mark PIT satisfied
      pitEntry->isSatisfied = true;
      pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

      // Dead Nonce List insert if necessary (for out-record of inFace)
      this->insertDeadNonceList(*pitEntry, &inFace);

      // clear PIT entry's in and out records
      //清除PIT条目
      // 这里清除的是PITentry的出入记录，并没有删除PITentry，这样做可以防止频繁地创建和销毁PITentry
      // 下次请求同样的数据包时，pitEntry直接通过find获取即可，无需insert
      pitEntry->clearInRecords();
      pitEntry->deleteOutRecord(inFace);
    }

    // foreach pending downstream
    for (Face* pendingDownstream : pendingDownstreams) {
      if (pendingDownstream->getId() == inFace.getId() &&
          pendingDownstream->getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
      // goto outgoing Data pipeline
      this->onOutgoingData(data, *pendingDownstream);
    }
  /*********************************send data to downstream*******************/
  }
}
/**********************************************************************************/
```
