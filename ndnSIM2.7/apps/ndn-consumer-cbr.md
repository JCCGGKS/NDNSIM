src/ndnSIM/apps/ndn-consumer-cbr.hpp
```cpp
namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 * 以“恒定”速率发出interest packet的申请(泊松过程)
 */
class ConsumerCbr : public Consumer {
public:
  static TypeId
  GetTypeId();

  /** Default constructor
   * Sets up randomizer function and packet sequence number
   * 设置随机的函数和包的序列号
   */
  ConsumerCbr();

  virtual ~ConsumerCbr();

protected:
  /**
   * Constructs the Interest packet and sends it using a callback to the underlying NDN protocol
   * 构造兴趣包，并使用回调将其发送到底层NDN协议
   */
  virtual void ScheduleNextPacket();

  // Set type of frequency randomization----设置频率随机化类型
  // value ： Either 'none', 'uniform', or 'exponential'
  // uniform:均匀分布是一种连续概率分布，其中随机变量在某个区间内的每一个值被取到的概率都是相等的。
  // exponential: 指数分布是一种连续概率分布，它描述了独立随机事件发生的时间间隔。指数分布是无记忆性的
  void SetRandomize(const std::string& value);

  /**
   * Get type of frequency randomization
   * @returns either 'none', 'uniform', or 'exponential'
   */
  std::string GetRandomize() const;

protected:
  double m_frequency; // Frequency of interest packets (in hertz)
  bool m_firstTime;
  Ptr<RandomVariableStream> m_random;
  std::string m_randomType;
};

} // namespace ndn
} // namespace ns3
```

src/ndnSIM/apps/ndn-consumer-cbr.cpp
```cpp
namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerCbr);

TypeId ConsumerCbr::GetTypeId(void){
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerCbr")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerCbr>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerCbr::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerCbr::SetRandomize, &ConsumerCbr::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerCbr::m_seqMax), MakeIntegerChecker<uint32_t>())

    ;

  return tid;
}

ConsumerCbr::ConsumerCbr()
  : m_frequency(1.0)
  , m_firstTime(true){
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

ConsumerCbr::~ConsumerCbr(){
}

void ConsumerCbr::ScheduleNextPacket(){
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Consumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &Consumer::SendPacket, this);
}

void ConsumerCbr::SetRandomize(const std::string& value){
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string ConsumerCbr::GetRandomize() const{
  return m_randomType;
}

} // namespace ndn
} // namespace ns3
```
