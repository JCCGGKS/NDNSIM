src/ndnSIM/apps/ndn-producer.hpp

```cpp
namespace ns3 {
namespace ndn {

/** ndn-apps
 *  A simple Interest-sink applia simple Interest-sink application
 *  一个简单的兴趣吸收(消费者)应用程序

 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.

  一个简单的兴趣接收器应用程序，它用指定大小和名称与兴趣相同的数据包应答每个传入的兴趣。
 */
class Producer : public App {
public:
  static TypeId GetTypeId(void);

  Producer();

/*******************************************on interest*****************************/
  // inherited from NdnApp
  virtual void OnInterest(shared_ptr<const Interest> interest);
/*******************************************on interest*****************************/
protected:
  // inherited from Application base class.
  virtual void StartApplication(); // Called at time specified by Start

  virtual void StopApplication(); // Called at time specified by Stop

private:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;
};

} // namespace ndn
} // namespace ns3
```


src/ndnSIM/apps/ndn-producer.cpp
```cpp
namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Producer);

TypeId Producer::GetTypeId(void){

  static TypeId tid =
    TypeId("ns3::ndn::Producer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<Producer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&Producer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&Producer::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&Producer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&Producer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&Producer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&Producer::m_keyLocator), MakeNameChecker());
  return tid;
}

Producer::Producer(){
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void Producer::StartApplication(){

  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  // 添加路由
  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void Producer::StopApplication(){
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}
/*******************************************on interest*****************************/
void Producer::OnInterest(shared_ptr<const Interest> interest){
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  // 用兴趣包的名称构造数据包的名称
  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  auto data = make_shared<Data>();
  // 设置数据包的名称
  data->setName(dataName);
  // 设置数据包新鲜度周期
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  // 设置内容
  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}
/*******************************************on interest*****************************/
} // namespace ndn
} // namespace ns3
```
