/NFD/daemon/fw/forwarder.hpp
```cpp
namespace nfd {

namespace fw {
class Strategy;
} // namespace fw

/** main class of NFD
 *  Forwarder owns all faces and tables, and implements forwarding pipelines.
 *  转发拥有所有的faces和tables，并且实现转发管道
 */
class Forwarder
{
public:
  Forwarder();

  VIRTUAL_WITH_TESTS
  ~Forwarder();

  const ForwarderCounters& getCounters() const{
    return m_counters;
  }

/******************************faces and policies*****************************/
public: // faces and policies
  FaceTable& getFaceTable(){
    return m_faceTable;
  }

  // get existing Face
  // shortcut(快捷方式) to .getFaceTable().get(face)
  Face* getFace(FaceId id) const{
    return m_faceTable.get(id);
  }


  // add new Face
  // shortcut to .getFaceTable().add(face)
  void addFace(shared_ptr<Face> face){
    m_faceTable.add(face);
  }


  fw::UnsolicitedDataPolicy& getUnsolicitedDataPolicy() const{
    return *m_unsolicitedDataPolicy;
  }

  void setUnsolicitedDataPolicy(unique_ptr<fw::UnsolicitedDataPolicy> policy){
    // 断言传入参数policy不为空
    BOOST_ASSERT(policy != nullptr);
    // 调用移动赋值函数，转移所有权
    m_unsolicitedDataPolicy = std::move(policy);
  }

public: // forwarding entrypoints and tables

  /** \brief start incoming Interest processing
   *  \param face ： face on which Interest is received
   *  \param interest ： the incoming Interest, must be well-formed(格式良好的) and created with make_shared
   */
  void startProcessInterest(Face& face, const Interest& interest){
    this->onIncomingInterest(face, interest);
  }

  /** \brief start incoming Data processing
   *  \param face ： face on which Data is received
   *  \param data ： the incoming Data, must be well-formed(格式良好的) and created with make_shared
   */
  void startProcessData(Face& face, const Data& data){
    this->onIncomingData(face, data);
  }

  /** \brief start incoming Nack processing
   *  \param face ： face on which Nack is received
   *  \param nack ： the incoming Nack, must be well-formed
   */
  void startProcessNack(Face& face, const lp::Nack& nack){
    this->onIncomingNack(face, nack);
  }

  NameTree& getNameTree(){
    return m_nameTree;
  }


  Fib& getFib(){
    return m_fib;
  }

  Pit& getPit(){
    return m_pit;
  }

  Cs& getCs(){
    return m_cs;
  }

  Measurements& getMeasurements(){
    return m_measurements;
  }

  StrategyChoice& getStrategyChoice(){
    return m_strategyChoice;
  }

  DeadNonceList& getDeadNonceList(){
    return m_deadNonceList;
  }

  NetworkRegionTable& getNetworkRegionTable(){
    return m_networkRegionTable;
  }
/******************************faces and policies*****************************/

public: // allow enabling ndnSIM content store (will be removed in the future)
//从2.8版本开始，老版本的Content Store被移除
  void setCsFromNdnSim(ns3::Ptr<ns3::ndn::ContentStore> cs){
    m_csFromNdnSim = cs;
  }

public:

  // trigger before PIT entry is satisfied
  // \sa Strategy::beforeSatisfyInterest
  signal::Signal<Forwarder, pit::Entry, Face, Data> beforeSatisfyInterest;

  // trigger before PIT entry expires
  // \sa Strategy::beforeExpirePendingInterest
  signal::Signal<Forwarder, pit::Entry> beforeExpirePendingInterest;

/************************************pipelines***********************************/

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // pipelines

  // incoming Interest pipeline
  VIRTUAL_WITH_TESTS void onIncomingInterest(Face& inFace, const Interest& interest);

  // Interest loop pipeline
  VIRTUAL_WITH_TESTS void onInterestLoop(Face& inFace, const Interest& interest);

  // Content Store miss pipeline
  VIRTUAL_WITH_TESTS void onContentStoreMiss(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest);


  // Content Store hit pipeline
  VIRTUAL_WITH_TESTS void
  onContentStoreHit(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                    const Interest& interest, const Data& data);

  // outgoing Interest pipeline
  VIRTUAL_WITH_TESTS void onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry, Face& outFace, const Interest& interest);

  // Interest finalize pipeline
  VIRTUAL_WITH_TESTS void onInterestFinalize(const shared_ptr<pit::Entry>& pitEntry);

  // incoming Data pipeline
  VIRTUAL_WITH_TESTS void onIncomingData(Face& inFace, const Data& data);

  // Data unsolicited pipeline
  VIRTUAL_WITH_TESTS void onDataUnsolicited(Face& inFace, const Data& data);

  // outgoing Data pipeline
  VIRTUAL_WITH_TESTS voidonOutgoingData(const Data& data, Face& outFace);

  // incoming Nack pipeline
  VIRTUAL_WITH_TESTS void onIncomingNack(Face& inFace, const lp::Nack& nack);

  // outgoing Nack pipeline
  VIRTUAL_WITH_TESTS void onOutgoingNack(const shared_ptr<pit::Entry>& pitEntry, const Face& outFace, const lp::NackHeader& nack);


  VIRTUAL_WITH_TESTS void onDroppedInterest(Face& outFace, const Interest& interest);
/************************************pipelines***********************************/


PROTECTED_WITH_TESTS_ELSE_PRIVATE:

  // set a new expiry timer (now + duration) on a PIT entry
  void setExpiryTimer(const shared_ptr<pit::Entry>& pitEntry, time::milliseconds duration);


  // insert Nonce to Dead Nonce List if necessary
  // upstream if null, insert Nonces from all out-records; if not null, insert Nonce only on the out-records of this face
  // upstream如果为空，从所有out-records插入Nonces；如果不为空，只在当前Face的out-records中插入Nonce
  VIRTUAL_WITH_TESTS void insertDeadNonceList(pit::Entry& pitEntry, Face* upstream);

  // call trigger (method) on the effective strategy of pitEntry
  // 对pitEntry的有效策略调用trigger (method)
#ifdef WITH_TESTS
  virtual void dispatchToStrategy(pit::Entry& pitEntry, std::function<void(fw::Strategy&)> trigger)
#else
  template<class Function>
  void dispatchToStrategy(pit::Entry& pitEntry, Function trigger)
#endif
  {
    trigger(m_strategyChoice.findEffectiveStrategy(pitEntry));
  }

private:
  // 声明在/NFD/daemon/fw/forwarder-counters.hpp
  ForwarderCounters m_counters;

  FaceTable m_faceTable;
  unique_ptr<fw::UnsolicitedDataPolicy> m_unsolicitedDataPolicy;

  NameTree           m_nameTree;
  Fib                m_fib;
  Pit                m_pit;
  Cs                 m_cs; //ndnSIM2.0及其以后实现的ContentStore
  Measurements       m_measurements;
  StrategyChoice     m_strategyChoice;
  DeadNonceList      m_deadNonceList;
  NetworkRegionTable m_networkRegionTable;
  shared_ptr<Face>   m_csFace;

  ns3::Ptr<ns3::ndn::ContentStore> m_csFromNdnSim;//ndnSIM1版本实现的ContentStore

  // allow Strategy (base class) to enter pipelines
  // 声明友类
  friend class fw::Strategy;
};

} // namespace nfd
```



/NFD/daemon/fw/forwarder.cpp
```cpp
namespace nfd {

NFD_LOG_INIT(Forwarder);

static Name getDefaultStrategyName(){
  return fw::BestRouteStrategy2::getStrategyName();
}

Forwarder::Forwarder()
  : m_unsolicitedDataPolicy(new fw::DefaultUnsolicitedDataPolicy())
  , m_fib(m_nameTree) // 委托构造函数
  , m_pit(m_nameTree)
  , m_measurements(m_nameTree)
  , m_strategyChoice(*this)
  , m_csFace(face::makeNullFace(FaceUri("contentstore://"))){

  getFaceTable().addReserved(m_csFace, face::FACEID_CONTENT_STORE);

  m_faceTable.afterAdd.connect([this] (Face& face) {
    face.afterReceiveInterest.connect(
      [this, &face] (const Interest& interest) {
        this->startProcessInterest(face, interest);
      });
    face.afterReceiveData.connect(
      [this, &face] (const Data& data) {
        this->startProcessData(face, data);
      });
    face.afterReceiveNack.connect(
      [this, &face] (const lp::Nack& nack) {
        this->startProcessNack(face, nack);
      });
    face.onDroppedInterest.connect(
      [this, &face] (const Interest& interest) {
        this->onDroppedInterest(face, interest);
      });
  });

  m_faceTable.beforeRemove.connect([this] (Face& face) {
    cleanupOnFaceRemoval(m_nameTree, m_fib, m_pit, face);
  });

  m_strategyChoice.setDefaultStrategy(getDefaultStrategyName());
}

// 默认析构函数
Forwarder::~Forwarder() = default;

/********************************************************************************************/
void Forwarder::onIncomingInterest(Face& inFace, const Interest& interest){
  // receive Interest
  NFD_LOG_DEBUG("onIncomingInterest face=" << inFace.getId() <<
                " interest=" << interest.getName());
  interest.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));
  ++m_counters.nInInterests;

  // /localhost scope control
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
    const_cast<Interest&>(interest).setForwardingHint({});
  }

  // PIT insert
  shared_ptr<pit::Entry> pitEntry = m_pit.insert(interest).first;


  // detect duplicate Nonce in PIT entry
  int dnw = fw::findDuplicateNonce(*pitEntry, interest.getNonce(), inFace);
  bool hasDuplicateNonceInPit = dnw != fw::DUPLICATE_NONCE_NONE;
  if (inFace.getLinkType() == ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    // for p2p face: duplicate Nonce from same incoming face is not loop
    hasDuplicateNonceInPit = hasDuplicateNonceInPit && !(dnw & fw::DUPLICATE_NONCE_IN_SAME);
  }
  if (hasDuplicateNonceInPit) {
    // goto Interest loop pipeline
    this->onInterestLoop(inFace, interest);
    return;
  }

  // is pending?
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
        this->onContentStoreHit(inFace, pitEntry, interest, *match);
      }
      else {
        this->onContentStoreMiss(inFace, pitEntry, interest);
      }
    }
  }
  else {
    this->onContentStoreMiss(inFace, pitEntry, interest);
  }
}
/********************************************************************************************/

void
Forwarder::onInterestLoop(Face& inFace, const Interest& interest)
{
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

static inline bool
compare_InRecord_expiry(const pit::InRecord& a, const pit::InRecord& b)
{
  return a.getExpiry() < b.getExpiry();
}

void
Forwarder::onContentStoreMiss(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                              const Interest& interest)
{
  NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName());
  ++m_counters.nCsMisses;

  // insert in-record
  pitEntry->insertOrUpdateInRecord(const_cast<Face&>(inFace), interest);

  // set PIT expiry timer to the time that the last PIT in-record expires
  auto lastExpiring = std::max_element(pitEntry->in_begin(), pitEntry->in_end(), &compare_InRecord_expiry);
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
      this->onOutgoingInterest(pitEntry, *nextHopFace, interest);
    }
    return;
  }

  // dispatch to strategy: after incoming Interest
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterReceiveInterest(inFace, interest, pitEntry); });
}

void
Forwarder::onContentStoreHit(const Face& inFace, const shared_ptr<pit::Entry>& pitEntry,
                             const Interest& interest, const Data& data)
{
  NFD_LOG_DEBUG("onContentStoreHit interest=" << interest.getName());
  ++m_counters.nCsHits;

  data.setTag(make_shared<lp::IncomingFaceIdTag>(face::FACEID_CONTENT_STORE));
  // XXX should we lookup PIT for other Interests that also match csMatch?

  pitEntry->isSatisfied = true;
  pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

  // set PIT expiry timer to now
  this->setExpiryTimer(pitEntry, 0_ms);

  beforeSatisfyInterest(*pitEntry, *m_csFace, data);
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, *m_csFace, data); });

  // dispatch to strategy: after Content Store hit
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterContentStoreHit(pitEntry, inFace, data); });
}

void
Forwarder::onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry, Face& outFace, const Interest& interest)
{
  NFD_LOG_DEBUG("onOutgoingInterest face=" << outFace.getId() <<
                " interest=" << pitEntry->getName());

  // insert out-record
  pitEntry->insertOrUpdateOutRecord(outFace, interest);

  // send Interest
  outFace.sendInterest(interest);
  ++m_counters.nOutInterests;
}

void
Forwarder::onInterestFinalize(const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("onInterestFinalize interest=" << pitEntry->getName() <<
                (pitEntry->isSatisfied ? " satisfied" : " unsatisfied"));

  if (!pitEntry->isSatisfied) {
    beforeExpirePendingInterest(*pitEntry);
  }

  // Dead Nonce List insert if necessary
  this->insertDeadNonceList(*pitEntry, 0);

  // Increment satisfied/unsatisfied Interests counter
  if (pitEntry->isSatisfied) {
    ++m_counters.nSatisfiedInterests;
  }
  else {
    ++m_counters.nUnsatisfiedInterests;
  }

  // PIT delete
  scheduler::cancel(pitEntry->expiryTimer);
  m_pit.erase(pitEntry.get());
}


//收到DataPacket
void
Forwarder::onIncomingData(Face& inFace, const Data& data)
{
  // receive Data
  NFD_LOG_DEBUG("onIncomingData face=" << inFace.getId() << " data=" << data.getName());
  data.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));
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

  // PIT match
  pit::DataMatchResult pitMatches = m_pit.findAllDataMatches(data);
  if (pitMatches.size() == 0) {
    // goto Data unsolicited pipeline
    //在PIT表中找不到对该数据包的请求条目，将其送入未请求通道
    this->onDataUnsolicited(inFace, data);
    return;
  }

  //DataPacket的数据副本
  shared_ptr<Data> dataCopyWithoutTag = make_shared<Data>(data);
  dataCopyWithoutTag->removeTag<lp::HopCountTag>();

  // CS insert
  //按照某种策略缓存该数据包
  //m_cs：2版本的实现的ContentStore
  //m_csFromNdnSim：1版本实现的ContentStore

  //默认的LCE策略
  if (m_csFromNdnSim == nullptr)
    m_cs.insert(*dataCopyWithoutTag);
  else
    m_csFromNdnSim->Add(dataCopyWithoutTag);

  //LCD策略
  auto lcdtag = dataCopyWithoutTag->getTag<lp::LcdTag>();
  if(lcdtag){
    uint64_t lcdvalue = lcdtag->get();
    //uint64_t lcdvalue = *lcdtag; //隐式类型转换； 使用static_cast可以进行显示类型转换
    if(lcdvalue == 0){
      if (m_csFromNdnSim == nullptr)
        m_cs.insert(*dataCopyWithoutTag);
      else
         m_csFromNdnSim->Add(dataCopyWithoutTag);
    }
  }





  // when only one PIT entry is matched, trigger strategy: after receive Data
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
  else {
    std::set<Face*> pendingDownstreams;
    auto now = time::steady_clock::now();

    for (const shared_ptr<pit::Entry>& pitEntry : pitMatches) {
      NFD_LOG_DEBUG("onIncomingData matching=" << pitEntry->getName());

      // remember pending downstreams
      for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
      //需要在兴趣包的过期时间以内收到数据包才可以
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
  }
}

void
Forwarder::onDataUnsolicited(Face& inFace, const Data& data)
{
  // accept to cache?
  fw::UnsolicitedDataDecision decision = m_unsolicitedDataPolicy->decide(inFace, data);
  if (decision == fw::UnsolicitedDataDecision::CACHE) {
    // CS insert
    if (m_csFromNdnSim == nullptr)
      m_cs.insert(data, true);
    else
      m_csFromNdnSim->Add(data.shared_from_this());
  }

  NFD_LOG_DEBUG("onDataUnsolicited face=" << inFace.getId() <<
                " data=" << data.getName() <<
                " decision=" << decision);
}

void
Forwarder::onOutgoingData(const Data& data, Face& outFace)
{
  if (outFace.getId() == face::INVALID_FACEID) {
    NFD_LOG_WARN("onOutgoingData face=invalid data=" << data.getName());
    return;
  }
  NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() << " data=" << data.getName());

  // /localhost scope control
  bool isViolatingLocalhost = outFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() <<
                  " data=" << data.getName() << " violates /localhost");
    // (drop)
    return;
  }

  // TODO traffic manager

  // send Data
  outFace.sendData(data);
  ++m_counters.nOutData;
}

void
Forwarder::onIncomingNack(Face& inFace, const lp::Nack& nack)
{
  // receive Nack
  nack.setTag(make_shared<lp::IncomingFaceIdTag>(inFace.getId()));
  ++m_counters.nInNacks;

  // if multi-access or ad hoc face, drop
  if (inFace.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onIncomingNack face=" << inFace.getId() <<
                  " nack=" << nack.getInterest().getName() <<
                  "~" << nack.getReason() << " face-is-multi-access");
    return;
  }

  // PIT match
  shared_ptr<pit::Entry> pitEntry = m_pit.find(nack.getInterest());
  // if no PIT entry found, drop
  if (pitEntry == nullptr) {
    NFD_LOG_DEBUG("onIncomingNack face=" << inFace.getId() <<
                  " nack=" << nack.getInterest().getName() <<
                  "~" << nack.getReason() << " no-PIT-entry");
    return;
  }

  // has out-record?
  pit::OutRecordCollection::iterator outRecord = pitEntry->getOutRecord(inFace);
  // if no out-record found, drop
  if (outRecord == pitEntry->out_end()) {
    NFD_LOG_DEBUG("onIncomingNack face=" << inFace.getId() <<
                  " nack=" << nack.getInterest().getName() <<
                  "~" << nack.getReason() << " no-out-record");
    return;
  }

  // if out-record has different Nonce, drop
  if (nack.getInterest().getNonce() != outRecord->getLastNonce()) {
    NFD_LOG_DEBUG("onIncomingNack face=" << inFace.getId() <<
                  " nack=" << nack.getInterest().getName() <<
                  "~" << nack.getReason() << " wrong-Nonce " <<
                  nack.getInterest().getNonce() << "!=" << outRecord->getLastNonce());
    return;
  }

  NFD_LOG_DEBUG("onIncomingNack face=" << inFace.getId() <<
                " nack=" << nack.getInterest().getName() <<
                "~" << nack.getReason() << " OK");

  // record Nack on out-record
  outRecord->setIncomingNack(nack);

  // set PIT expiry timer to now when all out-record receive Nack
  if (!fw::hasPendingOutRecords(*pitEntry)) {
    this->setExpiryTimer(pitEntry, 0_ms);
  }

  // trigger strategy: after receive NACK
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterReceiveNack(inFace, nack, pitEntry); });
}

void
Forwarder::onOutgoingNack(const shared_ptr<pit::Entry>& pitEntry, const Face& outFace,
                          const lp::NackHeader& nack)
{
  if (outFace.getId() == face::INVALID_FACEID) {
    NFD_LOG_WARN("onOutgoingNack face=invalid" <<
                  " nack=" << pitEntry->getInterest().getName() <<
                  "~" << nack.getReason() << " no-in-record");
    return;
  }

  // has in-record?
  pit::InRecordCollection::iterator inRecord = pitEntry->getInRecord(outFace);

  // if no in-record found, drop
  if (inRecord == pitEntry->in_end()) {
    NFD_LOG_DEBUG("onOutgoingNack face=" << outFace.getId() <<
                  " nack=" << pitEntry->getInterest().getName() <<
                  "~" << nack.getReason() << " no-in-record");
    return;
  }

  // if multi-access or ad hoc face, drop
  if (outFace.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onOutgoingNack face=" << outFace.getId() <<
                  " nack=" << pitEntry->getInterest().getName() <<
                  "~" << nack.getReason() << " face-is-multi-access");
    return;
  }

  NFD_LOG_DEBUG("onOutgoingNack face=" << outFace.getId() <<
                " nack=" << pitEntry->getInterest().getName() <<
                "~" << nack.getReason() << " OK");

  // create Nack packet with the Interest from in-record
  lp::Nack nackPkt(inRecord->getInterest());
  nackPkt.setHeader(nack);

  // erase in-record
  pitEntry->deleteInRecord(outFace);

  // send Nack on face
  const_cast<Face&>(outFace).sendNack(nackPkt);
  ++m_counters.nOutNacks;
}

void
Forwarder::onDroppedInterest(Face& outFace, const Interest& interest)
{
  m_strategyChoice.findEffectiveStrategy(interest.getName()).onDroppedInterest(outFace, interest);
}

void
Forwarder::setExpiryTimer(const shared_ptr<pit::Entry>& pitEntry, time::milliseconds duration)
{
  BOOST_ASSERT(pitEntry);
  BOOST_ASSERT(duration >= 0_ms);

  scheduler::cancel(pitEntry->expiryTimer);

  pitEntry->expiryTimer = scheduler::schedule(duration, [=] { onInterestFinalize(pitEntry); });
}

void
Forwarder::insertDeadNonceList(pit::Entry& pitEntry, Face* upstream)
{
  // need Dead Nonce List insert?
  bool needDnl = true;
  if (pitEntry.isSatisfied) {
    BOOST_ASSERT(pitEntry.dataFreshnessPeriod >= 0_ms);
    needDnl = static_cast<bool>(pitEntry.getInterest().getMustBeFresh()) &&
              pitEntry.dataFreshnessPeriod < m_deadNonceList.getLifetime();
  }

  if (!needDnl) {
    return;
  }

  // Dead Nonce List insert
  if (upstream == nullptr) {
    // insert all outgoing Nonces
    const auto& outRecords = pitEntry.getOutRecords();
    std::for_each(outRecords.begin(), outRecords.end(), [&] (const auto& outRecord) {
      m_deadNonceList.add(pitEntry.getName(), outRecord.getLastNonce());
    });
  }
  else {
    // insert outgoing Nonce of a specific face
    auto outRecord = pitEntry.getOutRecord(*upstream);
    if (outRecord != pitEntry.getOutRecords().end()) {
      m_deadNonceList.add(pitEntry.getName(), outRecord->getLastNonce());
    }
  }
}

} // namespace nfd
```
