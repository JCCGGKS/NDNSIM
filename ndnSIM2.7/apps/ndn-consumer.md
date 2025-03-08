ndnSIM/apps/ndn-consumer.hpp
```cpp
namespace ns3 {
namespace ndn {

/** ndn-apps
 * NDN application for sending out Interest packets
 * 发送兴趣包的NDN应用
 */
class Consumer : public App {
public:
  static TypeId GetTypeId();


  // 默认构造函数
  // 设置随机化功能和包序列号
  Consumer();

  virtual ~Consumer(){};

/***************************************************************************/
  // From App
  virtual void OnData(shared_ptr<const Data> contentObject);

  // From App
  virtual void OnNack(shared_ptr<const lp::Nack> nack);

  /** Timeout event --- 超时事件
   * @param sequenceNumber ： time outed sequence number---超时序列号
   */
  virtual void OnTimeout(uint32_t sequenceNumber);

  // Actually send packet
  // 实际发送数据包
  void SendPacket();
/***********************************************************************************/

  /**
   * An event that is fired just before an Interest packet is actually send out (send is inevitable)
     在Interest包实际发送之前触发的事件(发送是不可避免的)
   *
   * The reason for "before" even is that in certain cases (when it is possible to satisfy from the local cache),
   * the send call will immediately return data, and if "after" even was used, this after would be called after
   * all processing of incoming data, potentially producing unexpected results.
   */
  // 使用“before”even的原因是，在某些情况下(当可以从本地缓存中满足时)，send调用将立即返回数据，
  // 如果使用“after”even，则在所有传入数据的处理之后调用该after，可能会产生意想不到的结果。
  virtual void WillSendOutInterest(uint32_t sequenceNumber);

public:
  typedef void (*LastRetransmittedInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);
  typedef void (*FirstInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, uint32_t retxCount, int32_t hopCount);

protected:
  // from App
  virtual void StartApplication();

  virtual void StopApplication();


  // 构造兴趣包，并使用回调将其发送到底层NDN协议
  virtual void ScheduleNextPacket() = 0;


  // 检查数据包是否因为重传定时器到期而需要重传
  void CheckRetxTimeout();

  /**
   * \brief Modifies the frequency of checking the retransmission timeouts
   * \param retxTimer Timeout defining how frequent retransmission timeouts should be checked
   */
   /**
    * \brief 修改超时重传检查频率
    * \param retxTimer ： 超时，定义超时重传的检查频率
    */
  void SetRetxTimer(Time retxTimer);

  /**
   * \brief Returns the frequency of checking the retransmission timeouts
   * \return Timeout defining how frequent retransmission timeouts should be checked
   */
   /**
    * \brief 返回检查超时重传的频率
    * \return 超时，定义超时重传的检查频率
    */
  Time GetRetxTimer() const;

protected:
  Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator

  uint32_t m_seq;      ///< @brief currently requested sequence number---当前请求的序列号
  uint32_t m_seqMax;   ///< @brief maximum number of sequence number---序列号的最大值
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event---EventId待定的“发送包”事件
  Time m_retxTimer;    ///< @brief Currently estimated retransmission timer---当前预估的重传时间
  EventId m_retxEvent; ///< @brief Event to check whether or not retransmission should be performed---检查是否应该进行重传的事件

  Ptr<RttEstimator> m_rtt; ///< @brief RTT estimator---预估的RTT(Round trip time,往返时延)

  Time m_offTime;          ///< \brief Time interval between packets---包之间的时间间隔
  Name m_interestName;     ///< \brief NDN Name of the Interest (use Name)---兴趣包的名称
  Time m_interestLifeTime; ///< \brief LifeTime for interest packet---兴趣包的生存周期

  /// @cond include_hidden
  // 该结构体包含要重传的数据包的序列号
  struct RetxSeqsContainer : public std::set<uint32_t> {
  };

  // 要重传的有序序列号集合
  RetxSeqsContainer m_retxSeqs; // ordered set of sequence numbers to be retransmitted

  // 该结构体包含：数据包序列号及其超时时间
  struct SeqTimeout {
    SeqTimeout(uint32_t _seq, Time _time)
      : seq(_seq), time(_time){
    }

    uint32_t seq;
    Time time;
  };
  /// @endcond

  /// @cond include_hidden
  class i_seq {
  };
  class i_timestamp {
  };
  /// @endcond

  /// @cond include_hidden
  // 该结构体包含SeqTimeout结构体集合的多索引
  struct SeqTimeoutsContainer
    : public boost::multi_index::
        multi_index_container<SeqTimeout,
                              boost::multi_index::
                                indexed_by<boost::multi_index::
                                             ordered_unique<boost::multi_index::tag<i_seq>,
                                                            boost::multi_index::
                                                              member<SeqTimeout, uint32_t,
                                                                     &SeqTimeout::seq>>,
                                           boost::multi_index::
                                             ordered_non_unique<boost::multi_index::
                                                                  tag<i_timestamp>,
                                                                boost::multi_index::
                                                                  member<SeqTimeout, Time,
                                                                         &SeqTimeout::time>>>> {
  };

  SeqTimeoutsContainer m_seqTimeouts; // multi-index for the set of SeqTimeout structs---SeqTimeout结构集的多索引

  SeqTimeoutsContainer m_seqLastDelay;
  SeqTimeoutsContainer m_seqFullDelay;
  std::map<uint32_t, uint32_t> m_seqRetxCounts;


  // LastDelay表示DelayS和DelayUS表示最后一次发送的兴趣包和收到的数据包之间的延迟
  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */, int32_t /*hop count*/>
    m_lastRetransmittedInterestDataDelay;

  // FullDelay表示DelayS和DelayUS表示第一个兴趣包发送和接收到的数据包之间的延迟（即包括兴趣包重传时间）
  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */,
                 uint32_t /*retx count*/, int32_t /*hop count*/> m_firstInterestDataDelay;

  /// @endcond
};

} // namespace ndn
} // namespace ns3
```


ndnSIM/apps/ndn-consumer.cpp
```cpp
namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Consumer);

TypeId Consumer::GetTypeId(void){
  static TypeId tid =
    TypeId("ns3::ndn::Consumer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),/*序列号从0开始*/
                    MakeIntegerAccessor(&Consumer::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&Consumer::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),/*默认的生存周期是2s*/
                    MakeTimeAccessor(&Consumer::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),/*应该多久检查一次重传超时*/
                    MakeTimeAccessor(&Consumer::GetRetxTimer, &Consumer::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&Consumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::Consumer::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&Consumer::m_firstInterestDataDelay),
                      "ns3::ndn::Consumer::FirstInterestDataDelayCallback");

  return tid;
}

Consumer::Consumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything{

  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void Consumer::SetRetxTimer(Time retxTimer){
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events---取消任何计划的清理事件
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &Consumer::CheckRetxTimeout, this);
}

Time Consumer::GetRetxTimer() const{
  return m_retxTimer;
}

void Consumer::CheckRetxTimeout(){
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &Consumer::CheckRetxTimeout, this);
}

// Application Methods
void Consumer::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();

  ScheduleNextPacket();
}

void Consumer::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

/*********************************SendPacket*************************************/
// 发送兴趣包
void Consumer::SendPacket(){
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  // 在重传序列号中找到一个可用的序列号
  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    // 容器中有元素的情况下，删除第一个
    break;
  }

  // m_retxSeqs容器中没有元素，没有找到可用的序列号
  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = m_seq++;
  }

  // name with sequence
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendSequenceNumber(seq);

/****************************creat interest*************************************/
  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  // 随机生成的Nonce，范围在[0,max]
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  // 设置兴趣包的生存周期
  //[官方文档中的说明]应用程序设置InterestLifetime的值。如果省略InterestLifetime元素，则使用默认值 4 秒 (4000)。
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);

  // 发送interest
  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
}
/*********************************SendPacket*************************************/



///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

/*********************************OnData*************************************/
void Consumer::OnData(shared_ptr<const Data> data){
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  // hopCount
  int hopCount = 0;
  // 获取HopCountTag
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  // 插入缓存中数据包去除了HopCountTag字段[源码在src/ndnSIM/NFD/daemon/fw/forwarder.cpp#onIncomingData]
  // 缓存副本去除HopCountTag字段是为了命中时重新计算
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  // 判断兴趣是否是第一次发送
  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);

  // 在超时容器中找到，说明不是第一次发送，是重传兴趣
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  // 没找到，是第一次发送兴趣
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  // 成功收到data，回应ACK
  m_rtt->AckSeq(SequenceNumber32(seq));
}
/*********************************OnData*************************************/



/*********************************OnNack*************************************/
void Consumer::OnNack(shared_ptr<const lp::Nack> nack){
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}
/*********************************OnNack*************************************/


/*********************************OnTimeout***********************************/
// 超时重传
void Consumer::OnTimeout(uint32_t sequenceNumber){
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  ScheduleNextPacket();
}
/*********************************OnTimeout***********************************/


void Consumer::WillSendOutInterest(uint32_t sequenceNumber){
  NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  // 序列号对应的重传次数
  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}

} // namespace ndn
} // namespace ns3
```
