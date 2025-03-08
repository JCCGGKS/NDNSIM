src/ndnSIM/apps/ndn-consumer-zipf-mandelbrot.hpp

[Zipf-Mandelbrot](http://en.wikipedia.org/wiki/Zipf%E2%80%93Mandelbrot_law)
```cpp
namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * NDN app requesting contents following Zipf-Mandelbrot Distbituion----请求内容遵循齐夫分布
 *
 * The class implements an app which requests contents following Zipf-Mandelbrot Distribution
 * 该类实现了一个应用程序，该应用程序请求遵循Zipf-Mandelbrot分布的内容
 *
 * Here is the explaination of Zipf-Mandelbrot Distribution:
 * http://en.wikipedia.org/wiki/Zipf%E2%80%93Mandelbrot_law
 */
class ConsumerZipfMandelbrot : public ConsumerCbr {
public:
  static TypeId GetTypeId();

  /** Default constructor
   * Sets up randomized Number Generator (RNG)
   * Note: m_seq of its parent class ConsumerCbr here is used to record the interest number
   */
  ConsumerZipfMandelbrot();
  virtual ~ConsumerZipfMandelbrot();

  virtual void SendPacket();

  uint32_t GetNextSeq();

protected:
  virtual void ScheduleNextPacket();

private:
  void SetNumberOfContents(uint32_t numOfContents);

  uint32_t GetNumberOfContents() const;

  void SetQ(double q);

  double GetQ() const;

  void SetS(double s);

  double GetS() const;

private:
  uint32_t m_N;               // number of the contents
  double m_q;                 // q in (k+q)^s
  double m_s;                 // s in (k+q)^s
  std::vector<double> m_Pcum; // cumulative probability----累积概率

  Ptr<UniformRandomVariable> m_seqRng; // RNG
};

} /* namespace ndn */
} /* namespace ns3 */
```


src/ndnSIM/apps/ndn-consumer-zipf-mandelbrot.hpp
```cpp
namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerZipfMandelbrot);

TypeId ConsumerZipfMandelbrot::GetTypeId(void){
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerZipfMandelbrot")
      .SetGroupName("Ndn")
      .SetParent<ConsumerCbr>()
      .AddConstructor<ConsumerZipfMandelbrot>()

      .AddAttribute("NumberOfContents", "Number of the Contents in total", StringValue("100"),
                    MakeUintegerAccessor(&ConsumerZipfMandelbrot::SetNumberOfContents,
                                         &ConsumerZipfMandelbrot::GetNumberOfContents),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("q", "parameter of improve rank", StringValue("0.7"),
                    MakeDoubleAccessor(&ConsumerZipfMandelbrot::SetQ,
                                       &ConsumerZipfMandelbrot::GetQ),
                    MakeDoubleChecker<double>())

      .AddAttribute("s", "parameter of power", StringValue("0.7"),
                    MakeDoubleAccessor(&ConsumerZipfMandelbrot::SetS,
                                       &ConsumerZipfMandelbrot::GetS),
                    MakeDoubleChecker<double>());

  return tid;
}

ConsumerZipfMandelbrot::ConsumerZipfMandelbrot()
  : m_N(100) // needed here to make sure when SetQ/SetS are called, there is a valid value of N
  , m_q(0.7)
  , m_s(0.7) // 齐夫分布的指数
  , m_seqRng(CreateObject<UniformRandomVariable>()){
  // SetNumberOfContents is called by NS-3 object system during the initialization
}

ConsumerZipfMandelbrot::~ConsumerZipfMandelbrot(){
}

void ConsumerZipfMandelbrot::SetNumberOfContents(uint32_t numOfContents){

  m_N = numOfContents;

  NS_LOG_DEBUG(m_q << " and " << m_s << " and " << m_N);

  m_Pcum = std::vector<double>(m_N + 1);

  m_Pcum[0] = 0.0;
  for (uint32_t i = 1; i <= m_N; i++) {
    // 1.0/(r+q)^s
    m_Pcum[i] = m_Pcum[i - 1] + 1.0 / std::pow(i + m_q, m_s);
  }

  for (uint32_t i = 1; i <= m_N; i++) {
    // 归一化处理
    m_Pcum[i] = m_Pcum[i] / m_Pcum[m_N];
    NS_LOG_LOGIC("Cumulative probability [" << i << "]=" << m_Pcum[i]);
  }
}

uint32_t ConsumerZipfMandelbrot::GetNumberOfContents() const{
  return m_N;
}

void ConsumerZipfMandelbrot::SetQ(double q){
  m_q = q;
  SetNumberOfContents(m_N);
}

double ConsumerZipfMandelbrot::GetQ() const{
  return m_q;
}

void ConsumerZipfMandelbrot::SetS(double s){
  m_s = s;
  SetNumberOfContents(m_N);
}

double ConsumerZipfMandelbrot::GetS() const{
  return m_s;
}

void ConsumerZipfMandelbrot::SendPacket(){
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s max -> " << m_seqMax << "\n";

  // 在重传序列号中找到一个可用的序列号
  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());

    // NS_ASSERT (m_seqLifetimes.find (seq) != m_seqLifetimes.end ());
    // if (m_seqLifetimes.find (seq)->time <= Simulator::Now ())
    //   {

    //     NS_LOG_DEBUG ("Expire " << seq);
    //     m_seqLifetimes.erase (seq); // lifetime expired. Trying to find another unexpired
    //     sequence number
    //     continue;
    //   }
    NS_LOG_DEBUG("=interest seq " << seq << " from m_retxSeqs");
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) // no retransmission
  {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = ConsumerZipfMandelbrot::GetNextSeq();
    m_seq++;
  }

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s -> " << seq << "\n";

/**************************************create Interest*********************************/
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendSequenceNumber(seq);
  //

  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq << ", Total: " << m_seq << ", face: " << m_face->getId());
  NS_LOG_DEBUG("Trying to add " << seq << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(seq, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(seq, Simulator::Now()));

  m_seqLastDelay.erase(seq);
  m_seqLastDelay.insert(SeqTimeout(seq, Simulator::Now()));

  m_seqRetxCounts[seq]++;

  m_rtt->SentSeq(SequenceNumber32(seq), 1);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ConsumerZipfMandelbrot::ScheduleNextPacket();
/**************************************create Interest*********************************/
}

uint32_t ConsumerZipfMandelbrot::GetNextSeq(){
  uint32_t content_index = 1; //[1, m_N]
  double p_sum = 0;

  double p_random = m_seqRng->GetValue();
  while (p_random == 0) {
    p_random = m_seqRng->GetValue();
  }
  // if (p_random == 0)
  NS_LOG_LOGIC("p_random=" << p_random);
  // 好像轮盘赌算法，p_sum记录的是一个累积概率，哪个内容的累计概率刚好包裹住p_random就选择哪个内容
  for (uint32_t i = 1; i <= m_N; i++) {
    p_sum = m_Pcum[i]; // m_Pcum[i] = m_Pcum[i-1] + p[i], p[0] = 0;   e.g.: p_cum[1] = p[1],
                       // p_cum[2] = p[1] + p[2]
    if (p_random <= p_sum) {
      content_index = i;
      break;
    } // if
  }   // for
  // content_index = 1;
  NS_LOG_DEBUG("RandomNumber=" << content_index);
  return content_index;
}

void ConsumerZipfMandelbrot::ScheduleNextPacket(){

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &ConsumerZipfMandelbrot::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &ConsumerZipfMandelbrot::SendPacket, this);
}

} /* namespace ndn */
} /* namespace ns3 */
```

## 齐夫分布的核心实现

### SetNumberOfContents

```cpp
void ConsumerZipfMandelbrot::SetNumberOfContents(uint32_t numOfContents){

  m_N = numOfContents;

  NS_LOG_DEBUG(m_q << " and " << m_s << " and " << m_N);

  m_Pcum = std::vector<double>(m_N + 1);

  // 计算累积概率
  m_Pcum[0] = 0.0;
  for (uint32_t i = 1; i <= m_N; i++) {
    // 1.0/(r+q)^s
    m_Pcum[i] = m_Pcum[i - 1] + 1.0 / std::pow(i + m_q, m_s);
  }

  for (uint32_t i = 1; i <= m_N; i++) {
    // 归一化处理
    m_Pcum[i] = m_Pcum[i] / m_Pcum[m_N];
    NS_LOG_LOGIC("Cumulative probability [" << i << "]=" << m_Pcum[i]);
  }
}
```

### 前缀和的集合
给定一个整数数组，返回一个数组，其中每个元素是原数组从开始到当前元素的前缀和。

```cpp

```

### GetNextSeq

```cpp
uint32_t ConsumerZipfMandelbrot::GetNextSeq(){
  uint32_t content_index = 1; //[1, m_N]
  double p_sum = 0;

  double p_random = m_seqRng->GetValue();
  while (p_random == 0) {
    p_random = m_seqRng->GetValue();
  }
  // if (p_random == 0)
  NS_LOG_LOGIC("p_random=" << p_random);
  // 好像轮盘赌算法，p_sum记录的是一个累积概率，哪个内容的累计概率刚好包裹住p_random就选择哪个内容
  for (uint32_t i = 1; i <= m_N; i++) {
    p_sum = m_Pcum[i]; // m_Pcum[i] = m_Pcum[i-1] + p[i], p[0] = 0;   e.g.: p_cum[1] = p[1],
                       // p_cum[2] = p[1] + p[2]
    if (p_random <= p_sum) {
      content_index = i;
      break;
    } // if
  }   // for
  // content_index = 1;
  NS_LOG_DEBUG("RandomNumber=" << content_index);
  return content_index;
}
```
