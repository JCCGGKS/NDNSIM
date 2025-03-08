# 介绍一下接收网络兴趣包的流程(the process of receiving network-layer packet)

src/ndnSIM/NFD/daemon/fw/forwarder.cpp

## interest

## startProcessInterest
```cpp
/** start incoming Interest processing
 *  face : face on which Interest is received
 *  interest : the incoming Interest, must be well-formed and created with make_shared
 */
void startProcessInterest(Face& face, const Interest& interest){
  this->onIncomingInterest(face, interest);
}
```
## onIncomingInterest
```cpp
void Forwarder::onIncomingInterest(Face& inFace, const Interest& interest){
  // receive Interest
  // 记录兴趣包的传入接口ID和名称
  NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                " interest=" << interest.getName());

  // 设置兴趣包的IncomingFaceIdTag
  interest.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));
  ++m_counters.nInInterests;

  // localhost scope control
  bool isViolatingLocalhost = inFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(interest.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                  " interest=" << interest.getName() << " violates /localhost");
    // (drop)
    return;
  }

  // detect duplicate Nonce with Dead Nonce List
  /** Dead Nonce list
  * Dead Nonce List是一个全局表，它补充了PIT用于循环检测。当Nonce从PIT条目中被擦除(dead)时，Nonce和interest name被添加到Dead Nonce list中，
  * 并保留一段时间，在此期间预计会发生循环。为了减少内存使用，Interest Name和Nonce被存储为64位hash。
  * 可能会有误报(非循环的兴趣被认为是循环的)，但概率很小，并且当消费者使用不同的Nonce重传时，错误是可以恢复的。
  * 为了减少内存使用，条目没有关联的时间戳。相反，条目的生存期是通过动态调整容器容量来控制的。
  * 在固定的时间间隔内，将带有特殊值的条目MARK插入到容器中。存储在容器中的标记的数量反映了条目的生存期，因为以固定的间隔插入标记。
  */
  bool hasDuplicateNonceInDnl = m_deadNonceList.has(interest.getName(), interest.getNonce());
  if (hasDuplicateNonceInDnl) {
    // goto Interest loop pipeline
    this->onInterestLoop(inFace, interest);
    // 检测到循环，直接退出该函数
    return;
  }

  // strip forwarding hint if Interest has reached producer region
  // 如果兴趣包已到达生产者(数据源节点)，则直接转发提示
  if (!interest.getForwardingHint().empty() &&
      m_networkRegionTable.isInProducerRegion(interest.getForwardingHint())) {
    NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                  " interest=" << interest.getName() << " reaching-producer-region");
    // 设置转发提示
    // 传入的参数interest是常量对象，setForwardingHint是非常量成员函数
    // 先用const_cast强制去掉interest的底层const属性才能调用setForwardingHint
    const_cast<Interest&>(interest).setForwardingHint({});
  }

  // PIT insert
  // m_pit.insert返回std::pair<shared_ptr<Entry>, bool>，只获取第一个元素
  // 获取的pitEntry有可能是已经存在的，也有可能是新插入的
  shared_ptr<pit::Entry> pitEntry = m_pit.insert(interest).first;


  // detect duplicate Nonce in PIT entry
  // findDuplicateNonce 声明在/NFD/daemon/fw/algorithm.hpp----确定pitEntry是否有重复的Nonce“或”DuplicateNonceWhere
  int dnw = fw::findDuplicateNonce(*pitEntry, interest.getNonce(), inFace);
  bool hasDuplicateNonceInPit = dnw != fw::DUPLICATE_NONCE_NONE;

  if (inFace.getLinkType() == ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    // for p2p face: duplicate Nonce from same incoming face is not loop
    // 对于p2p face：相同incoming face的重复Nonce不是循环
    hasDuplicateNonceInPit = hasDuplicateNonceInPit && !(dnw & fw::DUPLICATE_NONCE_IN_SAME);
  }
  if (hasDuplicateNonceInPit) {
    // goto Interest loop pipeline
    this->onInterestLoop(inFace, interest);
    // 检测到循环直接退出
    return;
  }

  // is pending?判断是否是未决兴趣包
  // InRecords记录的是相同兴趣包的InFace，如果没有InRecords但是有pitEntry，
  // 说明该兴趣包之前被满足过对应的数据包有可能已经被缓存，转发之前先查询缓存
  if (!pitEntry->hasInRecords()) {
    if (m_csFromNdnSim == nullptr) {
    //在新版CS中查询缓存
    // bind用于参数绑定，std::ref返回引用对象，详解可见/C++/lambda.md
      m_cs.find(interest,
                bind(&Forwarder::onContentStoreHit, this, std::ref(inFace), pitEntry, _1, _2),
                bind(&Forwarder::onContentStoreMiss, this, std::ref(inFace), pitEntry, _1));
    }
    else {
    //在旧版Content Store中查询缓存
      shared_ptr<Data> match = m_csFromNdnSim->Lookup(interest.shared_from_this());
      if (match != nullptr) {
        // 缓存命中
        this->onContentStoreHit(inFace, pitEntry, interest, *match);
      }
      else {
        // 缓存失效
        this->onContentStoreMiss(inFace, pitEntry, interest);
      }
    }
  }
  else {
    // 有InRecords说明该兴趣包还没有被满足，需要转发到源节点获取对应的Data
    // 相对应的就是缓存失效
    this->onContentStoreMiss(inFace, pitEntry, interest);
  }
}
```
## onInterestLoop
在Dead Nonce list(补充PIT用于循环检测)检查到同样的interest name和Nonce(由于存储的是64bit的hash,所以可能存在误报：非循环的判定为循环)，进入loop pipelines
```cpp
void Forwarder::onInterestLoop(Face& inFace, const Interest& interest){

  // if multi-access or ad hoc face, drop
  // 如果inFace的链路类型不是点对点，记录日志并丢弃该packet
  if (inFace.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onInterestLoop face=" << inFace.getId() <<
                  " interest=" << interest.getName() <<
                  " drop");
    return;
  }

  // 发送Nack-duplicate
  NFD_LOG_DEBUG("onInterestLoop face=" << inFace.getId() <<
                " interest=" << interest.getName() <<
                " send-Nack-duplicate");

  // send Nack with reason=DUPLICATE
  // note: Don't enter outgoing Nack pipeline because it needs an in-record.
  lp::Nack nack(interest);
  // 发送Nack的原因是duplicate
  nack.setReason(lp::NackReason::DUPLICATE);
  inFace.sendNack(nack);
}
```

## 旧版本的缓存(hit or miss)
### hit---onContentStoreHit
```cpp
// inFace：兴趣包传入的接口
// pitEntry: 兴趣包在PIT中对应的表项
// data： 满足兴趣包的数据包
void Forwarder::onContentStoreHit(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                             const Interest& interest, const Data& data){

  // 记录缓存命中的兴趣包的名称
  NFD_LOG_DEBUG("onContentStoreHit interest=" << interest.getName());
  // 缓存命中次数递增
  ++m_counters.nCsHits;

  // 设置数据包的IncomingFaceIdTag
  data.setTag(make_shared<lp::IncomingFaceIdTag>(face::FACEID_CONTENT_STORE));
  // XXX should we lookup PIT for other Interests that also match csMatch?
  // 是否应该查询PIT满足其它请求同样数据包的兴趣包


  pitEntry->isSatisfied = true;
  pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

  // set PIT expiry timer to now
  // 因为PITEntry已经被满足，所以设置其过期时间为0，即现在就过期
  this->setExpiryTimer(pitEntry, 0_ms);

  // 在满足兴趣包之前执行的操作
  // signal::Signal<Forwarder, pit::Entry, Face, Data> beforeSatisfyInterest;
  beforeSatisfyInterest(*pitEntry, *m_csFace, data);

  /** strtategy
  * beforeSatisfyInterest和afterContentStoreHit声明在/NFD/daemon/fw/strategy.hpp
  * 这两个函数都是虚函数，由派生类具体实现
  */
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, *m_csFace, data); });

  // dispatch to strategy: after Content Store hit
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterContentStoreHit(pitEntry, inFace, data); });
}
```
### strategy.hpp的beforeSatisfyInterest和afterContentStoreHit
**beforeSatisfyInterest**
```cpp
void Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                const Face& inFace, const Data& data){

  // 记录相关信息
  NFD_LOG_DEBUG("beforeSatisfyInterest pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " data=" << data.getName());
}

```
**afterContentStoreHit**
```cpp
void Strategy::afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
                               const Face& inFace, const Data& data){
  // 记录相关信息                         
  NFD_LOG_DEBUG("afterContentStoreHit pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " data=" << data.getName());
  // 从inFace往下游发送数据包
  this->sendData(pitEntry, data, inFace);
}
```
### 具体策略的beforeSatisfyInterest和afterContentStoreHit
#### /NFD/daemon/fw/multicast-strategy.hpp
**beforeSatisfyInterest**
没有相应的具体实现，继承父类Strategy的beforeSatisfyInterest
**afterContentStoreHit**
没有相应的具体实现，继承父类Strategy的afterContentStoreHit

#### /NFD/daemon/fw/best-route-strategy.hpp
**beforeSatisfyInterest**
没有相应的具体实现，继承父类Strategy的beforeSatisfyInterest

**afterContentStoreHit**
没有相应的具体实现，继承父类Strategy的afterContentStoreHit


### miss---onContentStoreMiss
```cpp
void Forwarder::onContentStoreMiss(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                              const Interest& interest){
  // 记录缓存未命中的兴趣包的名称
  NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName());
  // 未命中次数递增
  ++m_counters.nCsMisses;

  // insert in-record
  // 在PIT Entry中记录兴趣包的传入接口inFace，要么更新要么插入
  pitEntry->insertOrUpdateInRecord(const_cast<Face&>(inFace), interest);

  // set PIT expiry timer to the time that the last PIT in-record expires
  // 设置PIT条目的过期时间为in-record中最大的过期时间，std::max_element详见/C++/std.md
  /** compare
  * static inline bool compare_InRecord_expiry(const pit::InRecord& a, const pit::InRecord& b){
  *   return a.getExpiry() < b.getExpiry();
  * }
  */
  auto lastExpiring = std::max_element(pitEntry->in_begin(), pitEntry->in_end(), &compare_InRecord_expiry);
  // 计算最后过期的生存间隔
  auto lastExpiryFromNow = lastExpiring->getExpiry() - time::steady_clock::now();
  //
  this->setExpiryTimer(pitEntry, time::duration_cast<time::milliseconds>(lastExpiryFromNow));

  // has NextHopFaceId?
  // NextHopFaceIdTag ： a packet tag for NextHopFaceId field
  shared_ptr<lp::NextHopFaceIdTag> nextHopTag = interest.getTag<lp::NextHopFaceIdTag>();
  if (nextHopTag != nullptr) {
    // chosen NextHop face exists?
    Face* nextHopFace = m_faceTable.get(*nextHopTag);
    if (nextHopFace != nullptr) {
      // 找到nextHopFace，记录缓存未命中兴趣包的名称和其转发的下一个接口
      NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName() << " nexthop-faceid=" << nextHopFace->getId());
      // go to outgoing Interest pipeline
      // scope control is unnecessary, because privileged app explicitly wants to forward
      // 从nextHopFace转发兴趣包
      this->onOutgoingInterest(pitEntry, *nextHopFace, interest);
    }
    return;
  }

  // 没有找到nextHopFace，依据具体的策略进行转发
  // dispatch to strategy: after incoming Interest
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterReceiveInterest(inFace, interest, pitEntry); });
}
```
### strategy.hpp的afterReceiveInterest
```cpp
virtual void afterReceiveInterest(const Face& inFace, const Interest& interest,
                      const shared_ptr<pit::Entry>& pitEntry) = 0;
```
这是一个纯虚函数，基类无法实现，必须由派生类实现。

### 具体策略的afterReceiveInterest
#### /NFD/daemon/fw/multicast-strategy.cpp
```cpp
void MulticastStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry){

  // 获取FIB entry
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  // 获取nexthop list
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  // 记录符合条件的nexthop的个数
  int nEligibleNextHops = 0;

  // 是否被抑制
  bool isSuppressed = false;

  // 遍历nexthops中的每一个nexthop
  for (const auto& nexthop : nexthops) {
    // 获取outFace
    Face& outFace = nexthop.getFace();

    // RetxSuppressionResult声明在/NFD/daemon/fw/retx-suppression.hpp，用来标识兴趣包的传输是否受到抑制
    // decidePerUpstream声明在/NFD/daemon/fw/retx-suppression-exponential.hpp，用来判断兴趣包的传输是否受到了抑制
    RetxSuppressionResult suppressResult = m_retxSuppression.decidePerUpstream(*pitEntry, outFace);

    // 兴趣包的传输受到了抑制
    if (suppressResult == RetxSuppressionResult::SUPPRESS) {
      NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                    << "to=" << outFace.getId() << " suppressed");
      isSuppressed = true;
      continue; // 下面的操作不再执行，直接遍历下一个nexthop
    }

    // outFace不满足条件
    if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
        wouldViolateScope(inFace, interest, outFace)) {
      continue;
    }

    // 从outFace向上游转发兴趣包
    this->sendInterest(pitEntry, outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " pitEntry-to=" << outFace.getId());

    if (suppressResult == RetxSuppressionResult::FORWARD) {
      m_retxSuppression.incrementIntervalForOutRecord(*pitEntry->getOutRecord(outFace));
    }
    // 符合条件的nexthop个数递增
    ++nEligibleNextHops;
  } // end for

  // 再没有抑制的情况下，找不到合适的nexthop
  if (nEligibleNextHops == 0 && !isSuppressed) {
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");

    // 设置nack原因并发送
    lp::NackHeader nackHeader;
    nackHeader.setReason(lp::NackReason::NO_ROUTE);
    this->sendNack(pitEntry, inFace, nackHeader);

    // 调度PIT条目以便立即删除
    // 这个辅助函数将PIT条目的过期时间设置为零。当策略认为兴趣无法转发并且不希望等待现有上游节点的响应时，应该调用此函数。
    this->rejectPendingInterest(pitEntry);
  }
}
```
#### /NFD/daemon/fw/best-route-strategy.hpp
```cpp
void BestRouteStrategyBase::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                            const shared_ptr<pit::Entry>& pitEntry){
  // 如果至少有一个输出记录等待Data，则为true                                        
  if (hasPendingOutRecords(*pitEntry)) {
    // not a new Interest, don't forward
    return;
  }

  // 获取fib entry
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  for (const auto& nexthop : fibEntry.getNextHops()) {
    Face& outFace = nexthop.getFace();
    // 找到一个满足条件的outFace发送兴趣包，只从一个outFace转发
    // canForwardToLegacy声明在/NFD/daemon/fw/algorithm.hpp
    if (!wouldViolateScope(inFace, interest, outFace) &&
        canForwardToLegacy(*pitEntry, outFace)) {
      this->sendInterest(pitEntry, outFace, interest);
      return;
    }
  }

  // 没有找到满足条件的outFace，
  this->rejectPendingInterest(pitEntry);
}
```

## 新版本的缓存(hit or miss)
```cpp
// cs.find
void Cs::find(const Interest& interest,
         const HitCallback& hitCallback,
         const MissCallback& missCallback) const{

  // 断言hitCallback 和 missCallback都不能为empty
  BOOST_ASSERT(static_cast<bool>(hitCallback));
  BOOST_ASSERT(static_cast<bool>(missCallback));

  // 如果缓存服务没开启或者空间剩余不足，判定为缓存未命中
  if (!m_shouldServe || m_policy->getLimit() == 0) {
    missCallback(interest);
    return;
  }

  const Name& prefix = interest.getName();
  bool isRightmost = interest.getChildSelector() == 1;
  NFD_LOG_DEBUG("find " << prefix << (isRightmost ? " R" : " L"));

  iterator first = m_table.lower_bound(prefix);
  iterator last = m_table.end();
  if (prefix.size() > 0) {
    last = m_table.lower_bound(prefix.getSuccessor());
  }

  iterator match = last;
  if (isRightmost) {
    match = this->findRightmost(interest, first, last);
  }
  else {
    match = this->findLeftmost(interest, first, last);
  }

  // 没有找到匹配项，缓存未命中
  if (match == last) {
    NFD_LOG_DEBUG("  no-match");
    missCallback(interest);
    return;
  }
  NFD_LOG_DEBUG("  matching " << match->getName());
  m_policy->beforeUse(match);
  // 找到匹配项缓存命中
  hitCallback(interest, match->getData());
}
```
### hit---hitCallback

```cpp
// 有关bind的详解见/C++/lambda.md
hitCallback : bind(&Forwarder::onContentStoreHit, this, std::ref(inFace), pitEntry, _1, _2)
```
调用hitCallback(_1,_2) 相当于调用 this->onContentStoreHit(inFace,pitEntry,_1,_2)

### miss---missCallback
```cpp
missCallback : bind(&Forwarder::onContentStoreMiss, this, std::ref(inFace), pitEntry, _1))
```
调用hitCallback(_1) 相当于调用 this->onContentStoreMiss(inFace,pitEntry,_1)
