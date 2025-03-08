/NFD/daemon/face/generic-link-service.hpp
```cpp
namespace nfd {
namespace face {

/** counters provided by GenericLinkService
 *  The type name 'GenericLinkServiceCounters' is implementation detail.
 *  Use 'GenericLinkService::Counters' in public API.
 */
// 由GenericLinkService提供的计数器
// 类型名称'GenericLinkServiceCounters'是实现细节。在公共API中使用'GenericLinkService::Counters'。
class GenericLinkServiceCounters : public virtual LinkService::Counters
{
public:

  // count of failed fragmentations
  // 分片失败的数量
  PacketCounter nFragmentationErrors;


  // count of outgoing LpPackets dropped due to exceeding MTU limit
  // If this counter is non-zero, the operator should enable fragmentation.
  // 由于超过MTU限制而被丢弃的lpPacket计数
  // 如果此计数器不为零，则应该启用分片。
  PacketCounter nOutOverMtu;

  // count of invalid LpPackets dropped before reassembly
  // 在重新组装之前被丢弃的无效的LpPackets的个数
  PacketCounter nInLpInvalid;

  // count of network-layer packets currently being reassembled
  // 当前正在重新组装的网络层数据包的个数
  SizeCounter<LpReassembler> nReassembling;

  // count of dropped partial network-layer packets due to reassembly timeout
  // 由于重新组装超时而被丢弃的部分网络层数据包个数
  PacketCounter nReassemblyTimeouts;

  // count of invalid reassembled network-layer packets dropped
  // 丢弃的无效重组网络层报文计数
  PacketCounter nInNetInvalid;

  // count of network-layer packets that did not require retransmission of a fragment
  // 不需要重传分片的网络层报文计数
  PacketCounter nAcknowledged;

  // count of network-layer packets that had at least one fragment retransmitted, but were eventually received in full
  // 至少有一个分片被重传，但最终被完整接收的网络层数据包的计数
  PacketCounter nRetransmitted;

  // count of network-layer packets dropped because a fragment reached the maximum number of retransmissions
  // 由于分片达到最大重传次数而丢弃的网络层报文计数
  PacketCounter nRetxExhausted;

  // count of outgoing LpPackets that were marked with congestion marks
  // 标记有拥塞标志的lpPackets的计数
  PacketCounter nCongestionMarked;
};
/*****************************************************************************************/

// GenericLinkService is a LinkService that implements the NDNLPv2 protocol
// GenericLinkService是一个实现NDNLPv2协议的LinkService
// \sa https://redmine.named-data.net/projects/nfd/wiki/NDNLPv2

class GenericLinkService : public LinkService
                         , protected virtual GenericLinkServiceCounters
{
public:
  /**************************Options*************************************/
  // Options that control the behavior of GenericLinkService
  class Options
  {
  public:
    constexpr Options() noexcept{
    }

  public:

    // enables encoding of IncomingFaceId, and decoding of NextHopFaceId and CachePolicy
    // 是否可以对IncomingFaceId编码，对NextHopFaceId和CachePolicy解码
    bool allowLocalFields = false;

    // enables fragmentation
    // 是否可以分片
    bool allowFragmentation = false;

    // options for fragmentation
    LpFragmenter::Options fragmenterOptions;

    // enables reassembly
    // 是否可以重新组装
    bool allowReassembly = false;

    // options for reassembly
    LpReassembler::Options reassemblerOptions;

    // options for reliability
    LpReliability::Options reliabilityOptions;

    // enables send queue congestion detection and marking
    // 是否启用拥塞检测和标记队列
    bool allowCongestionMarking = false;

    /** starting value for congestion marking interval
     *  The default value (100 ms) is taken from RFC 8289 (CoDel).
     */
    // 拥塞标记间隔的起始值
    // 默认值(100毫秒)取自RFC 8289 (CoDel)。
    time::nanoseconds baseCongestionMarkingInterval = 100_ms;


    /** default congestion threshold in bytes
     *  The default value (64 KiB) works well for a queue capacity of 200 KiB.
     */
    // 默认拥塞阈值，单位为字节
    // 默认值(64 KiB)适用于200 KiB的队列容量
    size_t defaultCongestionThreshold = 65536;

    // enables self-learning forwarding support
    // 是否允许自我学习转发支持
    bool allowSelfLearning = true;
  };
  /**************************Options*************************************/

  // counters provided by GenericLinkService
  using Counters = GenericLinkServiceCounters;

  // 显示构造函数
  // options = {} ： 列表初始化
  explicit GenericLinkService(const Options& options = {});


  // get Options used by GenericLinkService
  const Options& getOptions() const;

  // sets Options used by GenericLinkService
  void setOptions(const Options& options);

  const Counters& getCounters() const override;

/*******************************send path******************************************/

PROTECTED_WITH_TESTS_ELSE_PRIVATE: // send path

  // request an IDLE packet to transmit pending service fields
  void requestIdlePacket();

  // send an LpPacket fragment
  // pkt LpPacket to send
  void sendLpPacket(lp::Packet&& pkt);

  // send Interest
  void doSendInterest(const Interest& interest) override;

  // send Data
  void doSendData(const Data& data) override;

  // send Nack
  void doSendNack(const ndn::lp::Nack& nack) override;

private: // send path

  /** encode link protocol fields from tags onto an outgoing LpPacket
   *  \param netPkt ： network-layer packet to extract tags from
   *  \param lpPacket ： LpPacket to add link protocol fields to
   */
  // 将链路协议字段从tags编码到一个outgoing LpPacket
  // netPkt : 网络层包，从中提取tags
  // lpPacket： 向其中添加链路协议字段的LpPacket
  void encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket);

  /** send a complete network layer packet
   *  \param pkt ： LpPacket containing a complete network layer packet
   *  \param isInterest ： whether the network layer packet is an Interest
   */
  // 发送一个完整的网络层包
  // pkt： 包含一个完整的网络层包的LpPacket
  // isInterest ： 网络层包是否是一个兴趣包
  void sendNetPacket(lp::Packet&& pkt, bool isInterest);

  // assign a sequence number to an LpPacket
  // 为lpPacket指定序列号
  void assignSequence(lp::Packet& pkt);


  // assign consecutive sequence numbers to LpPackets
  // 为lpPackets分配连续的序列号
  void assignSequences(std::vector<lp::Packet>& pkts);


  /** if the send queue is found to be congested, add a congestion mark to the packet according to CoDel
   *  \sa https://tools.ietf.org/html/rfc8289
   */
  // 如果发现发送队列拥塞，则根据CoDel在数据包中添加拥塞标记
  void checkCongestionLevel(lp::Packet& pkt);
/*******************************send path******************************************/



/****************************************receive path*****************************/

private: // receive path

  // receive Packet from Transport
  void doReceivePacket(Transport::Packet&& packet) override;


  /** decode incoming network-layer packet
   *  \param netPkt ： reassembled network-layer packet
   *  \param firstPkt ： LpPacket of first fragment
   *
   *  If decoding is successful, a receive signal is emitted;otherwise, a warning is logged.
      如果解码成功，则发出receive信号;否则，将记录一个警告。
   */
  void decodeNetPacket(const Block& netPkt, const lp::Packet& firstPkt);

  /** decode incoming Interest
   *  \param netPkt ： reassembled network-layer packet; TLV-TYPE must be Interest
   *  \param firstPkt ： LpPacket of first fragment; must not have Nack field
   *
   *  If decoding is successful, receiveInterest signal is emitted;otherwise, a warning is logged.
      如果解码成功，则发出receiveInterest信号;否则，将记录一个警告。

   *  \throw tlv::Error ： parse error in an LpHeader field
   */
  void decodeInterest(const Block& netPkt, const lp::Packet& firstPkt);

  /** decode incoming Interest
   *  \param netPkt ： reassembled network-layer packet; TLV-TYPE must be Data
   *  \param firstPkt ： LpPacket of first fragment
   *
   *  If decoding is successful, receiveData signal is emitted;otherwise, a warning is logged.
      如果解码成功，则发出receiveData信号;否则，将记录一个警告。

   *  \throw tlv::Error ： parse error in an LpHeader field
   */
  void decodeData(const Block& netPkt, const lp::Packet& firstPkt);


  /** decode incoming Interest
   *  \param netPkt ： reassembled network-layer packet; TLV-TYPE must be Interest
                      重新组装的网络层包；TLV-TYPE必须是Interest
   *  \param firstPkt ： LpPacket of first fragment; must have Nack field
                          LpPacket的第一个分片；必须有Nack字段
   *
   *  If decoding is successful, receiveNack signal is emitted; otherwise, a warning is logged.
      如果解码成功，则发出receiveNack信号;否则，将记录一个警告。
   *
   *  \throw tlv::Error ： parse error in an LpHeader field
   */
  void decodeNack(const Block& netPkt, const lp::Packet& firstPkt);

PROTECTED_WITH_TESTS_ELSE_PRIVATE:
  Options m_options;
  LpFragmenter m_fragmenter;
  LpReassembler m_reassembler;
  LpReliability m_reliability;
  lp::Sequence m_lastSeqNo;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  // CongestionMark TLV-TYPE (3 octets) + CongestionMark TLV-LENGTH (1 octet) + sizeof(uint64_t)
  // 对于常量整型变量可以在类内初始化，但最好在类外定义。 详细介绍请见/C++/class/base.md/static
  static constexpr size_t CONGESTION_MARK_SIZE = 3 + 1 + sizeof(uint64_t);

  // Time to mark next packet due to send queue congestion
  // 标记下一个包的时间
  time::steady_clock::TimePoint m_nextMarkTime;

  // Time last packet was marked
  // 最后一个packet被标记的时间
  time::steady_clock::TimePoint m_lastMarkTime;

  // number of marked packets in the current incident of congestion
  // 当前拥塞事件中标记的报文数
  size_t m_nMarkedSinceInMarkingState;

  // 声明友类
  friend class LpReliability;
};
/**************************************************************************************/
inline const GenericLinkService::Options&
GenericLinkService::getOptions() const{
  return m_options;
}

inline const GenericLinkService::Counters&
GenericLinkService::getCounters() const{
  return *this;
}

} // namespace face
} // namespace nfd
```


/NFD/daemon/face/generic-link-service.cpp
```cpp
namespace nfd {
namespace face {

NFD_LOG_INIT(GenericLinkService);

constexpr uint32_t DEFAULT_CONGESTION_THRESHOLD_DIVISOR = 2;

GenericLinkService::GenericLinkService(const GenericLinkService::Options& options)
  : m_options(options)
  , m_fragmenter(m_options.fragmenterOptions, this)
  , m_reassembler(m_options.reassemblerOptions, this)
  , m_reliability(m_options.reliabilityOptions, this)
  , m_lastSeqNo(-2)
  , m_nextMarkTime(time::steady_clock::TimePoint::max())
  , m_lastMarkTime(time::steady_clock::TimePoint::min())
  , m_nMarkedSinceInMarkingState(0){

  m_reassembler.beforeTimeout.connect([this] (auto...) { ++this->nReassemblyTimeouts; });
  m_reliability.onDroppedInterest.connect([this] (const auto& i) { this->notifyDroppedInterest(i); });
  nReassembling.observe(&m_reassembler);
}

void GenericLinkService::setOptions(const GenericLinkService::Options& options){

  m_options = options;
  m_fragmenter.setOptions(m_options.fragmenterOptions);
  m_reassembler.setOptions(m_options.reassemblerOptions);
  m_reliability.setOptions(m_options.reliabilityOptions);
}

void GenericLinkService::requestIdlePacket(){

  // No need to request Acks to attach to this packet from LpReliability, as they are already attached in sendLpPacket
  // 不需要从LpReliability请求ack附加到这个数据包，因为它们已经在sendLpPacket中附加了
  this->sendLpPacket({});
}

void GenericLinkService::sendLpPacket(lp::Packet&& pkt){

  const ssize_t mtu = this->getTransport()->getMtu();

  if (m_options.reliabilityOptions.isEnabled) {
    m_reliability.piggyback(pkt, mtu);
  }

  if (m_options.allowCongestionMarking) {
    checkCongestionLevel(pkt);
  }

  Transport::Packet tp(pkt.wireEncode());
  if (mtu != MTU_UNLIMITED && tp.packet.size() > static_cast<size_t>(mtu)) {
    ++this->nOutOverMtu;
    // packet超出mtu大小需要分片
    NFD_LOG_FACE_WARN("attempted to send packet over MTU limit");
    return;
  }
  this->sendPacket(std::move(tp));
}

/************************************************************************/
/*最主要的两个函数：encodeLpFields和sendNetPacket*/

void GenericLinkService::doSendInterest(const Interest& interest){

  lp::Packet lpPacket(interest.wireEncode());

  encodeLpFields(interest, lpPacket);

  this->sendNetPacket(std::move(lpPacket), true);
}

void GenericLinkService::doSendData(const Data& data){
  lp::Packet lpPacket(data.wireEncode());

  encodeLpFields(data, lpPacket);

  this->sendNetPacket(std::move(lpPacket), false);
}

void GenericLinkService::doSendNack(const lp::Nack& nack){
  lp::Packet lpPacket(nack.getInterest().wireEncode());
  lpPacket.add<lp::NackField>(nack.getHeader());

  encodeLpFields(nack, lpPacket);

  this->sendNetPacket(std::move(lpPacket), false);
}
/********************************************************************************/




/****************************encodeLpFields and sendNetPacket**************************************/
// 从网络层包netPkt提取tags编码到链路层包lpPacket
void GenericLinkService::encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket){

  if (m_options.allowLocalFields) {
    shared_ptr<lp::IncomingFaceIdTag> incomingFaceIdTag = netPkt.getTag<lp::IncomingFaceIdTag>();
    if (incomingFaceIdTag != nullptr) {
      lpPacket.add<lp::IncomingFaceIdField>(*incomingFaceIdTag);
    }
  }

  shared_ptr<lp::CongestionMarkTag> congestionMarkTag = netPkt.getTag<lp::CongestionMarkTag>();
  if (congestionMarkTag != nullptr) {
    lpPacket.add<lp::CongestionMarkField>(*congestionMarkTag);
  }

  if (m_options.allowSelfLearning) {
    shared_ptr<lp::NonDiscoveryTag> nonDiscoveryTag = netPkt.getTag<lp::NonDiscoveryTag>();
    if (nonDiscoveryTag != nullptr) {
      lpPacket.add<lp::NonDiscoveryField>(*nonDiscoveryTag);
    }

    shared_ptr<lp::PrefixAnnouncementTag> prefixAnnouncementTag = netPkt.getTag<lp::PrefixAnnouncementTag>();
    if (prefixAnnouncementTag != nullptr) {
      lpPacket.add<lp::PrefixAnnouncementField>(*prefixAnnouncementTag);
    }
  }

  // HopCountTag
  //decode(receive)过程中，将hopcounttag增加1
  //encode(send)时什么也不做
  shared_ptr<lp::HopCountTag> hopCountTag = netPkt.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) {
    lpPacket.add<lp::HopCountTagField>(*hopCountTag);
  }
  else {
    lpPacket.add<lp::HopCountTagField>(0);
  }

}

void GenericLinkService::sendNetPacket(lp::Packet&& pkt, bool isInterest){

  std::vector<lp::Packet> frags;
  ssize_t mtu = this->getTransport()->getMtu();

  // Make space for feature fields in fragments
  if (m_options.reliabilityOptions.isEnabled && mtu != MTU_UNLIMITED) {
    mtu -= LpReliability::RESERVED_HEADER_SPACE;
  }

  if (m_options.allowCongestionMarking && mtu != MTU_UNLIMITED) {
    mtu -= CONGESTION_MARK_SIZE;
  }

  BOOST_ASSERT(mtu == MTU_UNLIMITED || mtu > 0);

  if (m_options.allowFragmentation && mtu != MTU_UNLIMITED) {
    bool isOk = false;
    std::tie(isOk, frags) = m_fragmenter.fragmentPacket(pkt, mtu);
    if (!isOk) {
      // 分片失败
      // fragmentation failed (warning is logged by LpFragmenter)
      ++this->nFragmentationErrors;
      return;
    }
  }
  else {
    if (m_options.reliabilityOptions.isEnabled) {
      frags.push_back(pkt);
    }
    else {
      frags.push_back(std::move(pkt));
    }
  }

  if (frags.size() == 1) {
    // even if indexed fragmentation is enabled, the fragmenter should not fragment the packet if it can fit in MTU
    // 如果包的大小没有超过MTU，即使开启了分片也不需要对包进行分片
    BOOST_ASSERT(!frags.front().has<lp::FragIndexField>());
    BOOST_ASSERT(!frags.front().has<lp::FragCountField>());
  }

  // Only assign sequences to fragments if packet contains more than 1 fragment
  // 如果包包含超过一个分片，为这些分片分配连续的序列号
  if (frags.size() > 1) {
    // Assign sequences to all fragments
    this->assignSequences(frags);
  }

  if (m_options.reliabilityOptions.isEnabled && frags.front().has<lp::FragmentField>()) {
    m_reliability.handleOutgoing(frags, std::move(pkt), isInterest);
  }

  for (lp::Packet& frag : frags) {
    this->sendLpPacket(std::move(frag));
  }
}
/****************************encodeLpFields and sendNetPacket**************************************/


void GenericLinkService::assignSequence(lp::Packet& pkt){
  pkt.set<lp::SequenceField>(++m_lastSeqNo);
}

void GenericLinkService::assignSequences(std::vector<lp::Packet>& pkts){
  // 以值的方式捕获this
  std::for_each(pkts.begin(), pkts.end(), [this] (auto& pkt) { this->assignSequence(pkt); });
}

void GenericLinkService::checkCongestionLevel(lp::Packet& pkt){
  ssize_t sendQueueLength = getTransport()->getSendQueueLength();
  // This operation requires that the transport supports retrieving current send queue length
  if (sendQueueLength < 0) {
    return;
  }

  // To avoid overflowing the queue, set the congestion threshold to at least half of the send
  // queue capacity.
  size_t congestionThreshold = m_options.defaultCongestionThreshold;
  if (getTransport()->getSendQueueCapacity() >= 0) {
    congestionThreshold = std::min(congestionThreshold,
                                   static_cast<size_t>(getTransport()->getSendQueueCapacity()) /
                                                       DEFAULT_CONGESTION_THRESHOLD_DIVISOR);
  }

  if (sendQueueLength > 0) {
    NFD_LOG_FACE_TRACE("txqlen=" << sendQueueLength << " threshold=" << congestionThreshold <<
                       " capacity=" << getTransport()->getSendQueueCapacity());
  }

  if (static_cast<size_t>(sendQueueLength) > congestionThreshold) { // Send queue is congested
    const auto now = time::steady_clock::now();
    if (now >= m_nextMarkTime || now >= m_lastMarkTime + m_options.baseCongestionMarkingInterval) {
      // Mark at most one initial packet per baseCongestionMarkingInterval
      if (m_nMarkedSinceInMarkingState == 0) {
        m_nextMarkTime = now;
      }

      // Time to mark packet
      pkt.set<lp::CongestionMarkField>(1);
      ++nCongestionMarked;
      NFD_LOG_FACE_DEBUG("LpPacket was marked as congested");

      ++m_nMarkedSinceInMarkingState;
      // Decrease the marking interval by the inverse of the square root of the number of packets
      // marked in this incident of congestion
      m_nextMarkTime += time::nanoseconds(static_cast<time::nanoseconds::rep>(
                                            m_options.baseCongestionMarkingInterval.count() /
                                            std::sqrt(m_nMarkedSinceInMarkingState)));
      m_lastMarkTime = now;
    }
  }
  else if (m_nextMarkTime != time::steady_clock::TimePoint::max()) {
    // Congestion incident has ended, so reset
    NFD_LOG_FACE_DEBUG("Send queue length dropped below congestion threshold");
    m_nextMarkTime = time::steady_clock::TimePoint::max();
    m_nMarkedSinceInMarkingState = 0;
  }
}

/********************************doReceivePacket************************************/
void GenericLinkService::doReceivePacket(Transport::Packet&& packet){
  try {
    lp::Packet pkt(packet.packet);

    if (m_options.reliabilityOptions.isEnabled) {
      m_reliability.processIncomingPacket(pkt);
    }

    if (!pkt.has<lp::FragmentField>()) {
      NFD_LOG_FACE_TRACE("received IDLE packet: DROP");
      return;
    }

    if ((pkt.has<lp::FragIndexField>() || pkt.has<lp::FragCountField>()) &&
        !m_options.allowReassembly) {
          // 重组失败丢弃
      NFD_LOG_FACE_WARN("received fragment, but reassembly disabled: DROP");
      return;
    }

    bool isReassembled = false;
    Block netPkt;
    lp::Packet firstPkt;
    std::tie(isReassembled, netPkt, firstPkt) = m_reassembler.receiveFragment(packet.remoteEndpoint,
                                                                              pkt);
    if (isReassembled) {
      // 解码网络包
      this->decodeNetPacket(netPkt, firstPkt);
    }
  }
  catch (const tlv::Error& e) {
    ++this->nInLpInvalid;
    NFD_LOG_FACE_WARN("packet parse error (" << e.what() << "): DROP");
  }
}

/***************************************deocode************************************************/
void GenericLinkService::decodeNetPacket(const Block& netPkt, const lp::Packet& firstPkt){
  try {
    switch (netPkt.type()) {
      case tlv::Interest: // 兴趣包
        if (firstPkt.has<lp::NackField>()) {
          this->decodeNack(netPkt, firstPkt);
        }
        else {
          this->decodeInterest(netPkt, firstPkt);
        }
        break;
      case tlv::Data: //数据包
        this->decodeData(netPkt, firstPkt);
        break;
      default:
        ++this->nInNetInvalid;
        // 无法识别网络层包类型
        NFD_LOG_FACE_WARN("unrecognized network-layer packet TLV-TYPE " << netPkt.type() << ": DROP");
        return;
    }
  }
  catch (const tlv::Error& e) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("packet parse error (" << e.what() << "): DROP");
  }
}

void GenericLinkService::decodeInterest(const Block& netPkt, const lp::Packet& firstPkt){

  BOOST_ASSERT(netPkt.type() == tlv::Interest);
  BOOST_ASSERT(!firstPkt.has<lp::NackField>());

  // forwarding expects Interest to be created with make_shared
  // 使用make_shared创建兴趣包
  auto interest = make_shared<Interest>(netPkt);

  // Increment HopCount
  // 增加HopCount
  if (firstPkt.has<lp::HopCountTagField>()) {
    interest->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    if (m_options.allowLocalFields) {
      interest->setTag(make_shared<lp::NextHopFaceIdTag>(firstPkt.get<lp::NextHopFaceIdField>()));
    }
    else {
      NFD_LOG_FACE_WARN("received NextHopFaceId, but local fields disabled: DROP");
      return;
    }
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received CachePolicy with Interest: DROP");
    return;
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    interest->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    if (m_options.allowSelfLearning) {
      interest->setTag(make_shared<lp::NonDiscoveryTag>(firstPkt.get<lp::NonDiscoveryField>()));
    }
    else {
      NFD_LOG_FACE_WARN("received NonDiscovery, but self-learning disabled: IGNORE");
    }
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received PrefixAnnouncement with Interest: DROP");
    return;
  }

  // 声明在/NFD/daemon/face/link-service.hpp
  this->receiveInterest(*interest);
}


void GenericLinkService::decodeData(const Block& netPkt, const lp::Packet& firstPkt){

  BOOST_ASSERT(netPkt.type() == tlv::Data);

  // forwarding expects Data to be created with make_shared
  // 使用make_shared创建数据包
  auto data = make_shared<Data>(netPkt);

  // 增加HopCount
  if (firstPkt.has<lp::HopCountTagField>()) {
    data->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  if (firstPkt.has<lp::NackField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received Nack with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NextHopFaceId with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    // CachePolicy is unprivileged and does not require allowLocalFields option.
    // In case of an invalid CachePolicyType, get<lp::CachePolicyField> will throw,
    // so it's unnecessary to check here.
    data->setTag(make_shared<lp::CachePolicyTag>(firstPkt.get<lp::CachePolicyField>()));
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    data->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NonDiscovery with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    if (m_options.allowSelfLearning) {
      data->setTag(make_shared<lp::PrefixAnnouncementTag>(firstPkt.get<lp::PrefixAnnouncementField>()));
    }
    else {
      NFD_LOG_FACE_WARN("received PrefixAnnouncement, but self-learning disabled: IGNORE");
    }
  }

  this->receiveData(*data);
}


void GenericLinkService::decodeNack(const Block& netPkt, const lp::Packet& firstPkt){

  BOOST_ASSERT(netPkt.type() == tlv::Interest);
  BOOST_ASSERT(firstPkt.has<lp::NackField>());

  lp::Nack nack((Interest(netPkt)));
  nack.setHeader(firstPkt.get<lp::NackField>());

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NextHopFaceId with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received CachePolicy with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    nack.setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NonDiscovery with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received PrefixAnnouncement with Nack: DROP");
    return;
  }

  this->receiveNack(nack);
}
/***************************************deocode************************************************/

} // namespace face
} // namespace nfd
```

send的流程：encodeLpFields--->sendNetPacket--->sendLpPacket--->sendPacket---->m_transport->send
receive的流程：doReceivePacket--->decodeNetPacket--->receive(Interest,Data,Nack)
