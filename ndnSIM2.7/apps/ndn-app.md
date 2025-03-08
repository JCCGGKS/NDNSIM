src/ndnSIM/apps/ndn-app.hpp
```cpp
namespace ns3 {

class Packet;

namespace ndn {


/** ndn-apps
 *
 * Base class that all NDN applications should be derived from.
 * 所有NDN应用程序都应该继承的基类
 * The class implements virtual calls onInterest, onNack, and onData
 * 这个类实现了虚函数onInterest , onNack, onData
 */
class App : public Application {
public:
  static TypeId GetTypeId();

  // Default constructor
  App();

  // 虚函数---析构函数
  virtual ~App();

  // Get application ID (ID of applications face)
  uint32_t GetId() const;
/*****************************************************************************/
  // Method that will be called every time new Interest arrives
  virtual void OnInterest(shared_ptr<const Interest> interest);

  // Method that will be called every time new Data arrives
  virtual void OnData(shared_ptr<const Data> data);

  // Method that will be called every time new Nack arrives
  virtual void OnNack(shared_ptr<const lp::Nack> nack);
/*****************************************************************************/

public:
  // InterestTraceCallback
  typedef void (*InterestTraceCallback)(shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>);
  // DataTraceCallback
  typedef void (*DataTraceCallback)(shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>);
  // NackTraceCallback
  typedef void (*NackTraceCallback)(shared_ptr<const lp::Nack>, Ptr<App>, shared_ptr<Face>);

protected:
  virtual void DoInitialize();

  virtual void DoDispose();

  // inherited from Application base class. Originally they were private
  // 从基类Application继承的函数，在Application中是私有的
  virtual void StartApplication(); ///< @brief Called at time specified by Start

  virtual void StopApplication(); ///< @brief Called at time specified by Stop

protected:
  bool m_active; ///< @brief Flag to indicate that application is active (set by StartApplication and StopApplication)
  shared_ptr<Face> m_face;
  AppLinkService* m_appLink;

  uint32_t m_appId;

  TracedCallback<shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>>
    m_receivedInterests; ///< @brief App-level trace of received Interests

  TracedCallback<shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>>
    m_receivedDatas; ///< @brief App-level trace of received Data

  TracedCallback<shared_ptr<const lp::Nack>, Ptr<App>, shared_ptr<Face>>
    m_receivedNacks; ///< @brief App-level trace of received Nacks

  TracedCallback<shared_ptr<const Interest>, Ptr<App>, shared_ptr<Face>>
    m_transmittedInterests; ///< @brief App-level trace of transmitted Interests

  TracedCallback<shared_ptr<const Data>, Ptr<App>, shared_ptr<Face>>
    m_transmittedDatas; ///< @brief App-level trace of transmitted Data

  TracedCallback<shared_ptr<const lp::Nack>, Ptr<App>, shared_ptr<Face>>
    m_transmittedNacks; ///< @brief App-level trace of transmitted Nacks
};

} // namespace ndn
} // namespace ns3
```




src/ndnSIM/apps/ndn-app.cpp
```cpp
namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(App);

TypeId App::GetTypeId(void){

  static TypeId tid = TypeId("ns3::ndn::App")
                        .SetGroupName("Ndn")
                        .SetParent<Application>()
                        .AddConstructor<App>()

                        .AddTraceSource("ReceivedInterests", "ReceivedInterests",
                                        MakeTraceSourceAccessor(&App::m_receivedInterests),
                                        "ns3::ndn::App::InterestTraceCallback")

                        .AddTraceSource("ReceivedDatas", "ReceivedDatas",
                                        MakeTraceSourceAccessor(&App::m_receivedDatas),
                                        "ns3::ndn::App::DataTraceCallback")

                        .AddTraceSource("ReceivedNacks", "ReceivedNacks",
                                        MakeTraceSourceAccessor(&App::m_receivedNacks),
                                        "ns3::ndn::App::NackTraceCallback")

                        .AddTraceSource("TransmittedInterests", "TransmittedInterests",
                                        MakeTraceSourceAccessor(&App::m_transmittedInterests),
                                        "ns3::ndn::App::InterestTraceCallback")

                        .AddTraceSource("TransmittedDatas", "TransmittedDatas",
                                        MakeTraceSourceAccessor(&App::m_transmittedDatas),
                                        "ns3::ndn::App::DataTraceCallback")

                        .AddTraceSource("TransmittedNacks", "TransmittedNacks",
                                        MakeTraceSourceAccessor(&App::m_transmittedNacks),
                                        "ns3::ndn::App::NackTraceCallback");
  return tid;
}

App::App()
  : m_active(false)
  , m_face(0)
  , m_appId(std::numeric_limits<uint32_t>::max()){
}

App::~App(){
}

void App::DoInitialize(){
  NS_LOG_FUNCTION_NOARGS();

  // find out what is application id on the node
  // 获取app ID
  for (uint32_t id = 0; id < GetNode()->GetNApplications(); ++id) {
    if (GetNode()->GetApplication(id) == this) {
      m_appId = id;
    }
  }

  Application::DoInitialize();
}

void App::DoDispose(void){
  NS_LOG_FUNCTION_NOARGS();

  // Unfortunately, this causes SEGFAULT;The best reason I see is that apps are freed after ndn stack is removed
  // 不幸的是，这会导致SEGFAULT;我看到的最好的原因是，在ndn堆栈被删除后，应用程序被释放
  // StopApplication ();
  Application::DoDispose();
}

uint32_t App::GetId() const{
  return m_appId;
}
/********************************************************************/
void App::OnInterest(shared_ptr<const Interest> interest){

  NS_LOG_FUNCTION(this << interest);
  m_receivedInterests(interest, this, m_face);
}

void App::OnData(shared_ptr<const Data> data){

  NS_LOG_FUNCTION(this << data);
  m_receivedDatas(data, this, m_face);
}

void App::OnNack(shared_ptr<const lp::Nack> nack){

  NS_LOG_FUNCTION(this << nack);

  // @TODO Implement
  m_receivedNacks(nack, this, m_face);
}
/*********************************************************************/

// 将m_active赋值为true
// Application Methods
void App::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT(m_active != true);
  m_active = true;

  // 节点应该已经安装NDN协议栈
  NS_ASSERT_MSG(GetNode()->GetObject<L3Protocol>() != 0,
                "Ndn stack should be installed on the node " << GetNode());

  // step 1. Create a face
  auto appLink = make_unique<AppLinkService>(this);
  auto transport = make_unique<NullTransport>("appFace://", "appFace://",
                                              ::ndn::nfd::FACE_SCOPE_LOCAL);
  // @TODO Consider making AppTransport instead
  m_face = std::make_shared<Face>(std::move(appLink), std::move(transport));
  m_appLink = static_cast<AppLinkService*>(m_face->getLinkService());
  m_face->setMetric(1);

  // step 2. Add face to the Ndn stack
  GetNode()->GetObject<L3Protocol>()->addFace(m_face);
}


// 将m_active赋值为false
void App::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  if (!m_active)
    return; // don't assert here, just return

  m_active = false;

  m_face->close();
}

} // namespace ndn
} // namespace ns3
```
