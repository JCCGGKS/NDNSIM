/NFD/daemon/face/link-service.hpp
```cpp
namespace nfd {
namespace face {

class Face;

/** counters provided by LinkService
 *  类型名'LinkServiceCounters'是实现细节。在公共API中使用'LinkService::Counters'。
 */
// PacketCounter声明定义在/NFD/core/counter.hpp
// 数据包级别的跟踪，详见https://ndnsim.net/2.7/metric.html#packet-level-trace-helpers
class LinkServiceCounters
{
public:

  // count of incoming Interests
  PacketCounter nInInterests;


  // count of outgoing Interests
  PacketCounter nOutInterests;


  // count of Interests dropped by reliability system for exceeding allowed number of retx
  PacketCounter nDroppedInterests;

  // count of incoming Data packets
  PacketCounter nInData;

  // count of outgoing Data packets
  PacketCounter nOutData;

  // count of incoming Nacks
  PacketCounter nInNacks;

  // count of outgoing Nacks
  PacketCounter nOutNacks;
};

/*********************************************************************************/

// the upper part of a Face
class LinkService : protected virtual LinkServiceCounters, noncopyable
{
public:

  // counters provided by LinkService
  typedef LinkServiceCounters Counters;

public:
  LinkService();


  // virtual function
  virtual  ~LinkService();


  // set Face and Transport for LinkService
  // 前提是setFaceAndTransport has not been called
  void setFaceAndTransport(Face& face, Transport& transport);


  // return Face to which this LinkService is attached
  const Face* getFace() const;


  // return Transport to which this LinkService is attached
  const Transport* getTransport() const;


  // return Transport to which this LinkService is attached
  Transport* getTransport();

  // virtual function
  virtual const Counters& getCounters() const;

public: // upper interface to be used by forwarding

/**************upper intereface to be used by forwarding********************/
  // send Interest
  // 前提是setTransport has been called
  void sendInterest(const Interest& interest);


  // send Data
  // 前提是setTransport has been called
  void sendData(const Data& data);


  // send Nack
  // 前提是setTransport has been called
  void sendNack(const ndn::lp::Nack& nack);
/**************upper intereface to be used by forwarding********************/

/********************************signals****************************************/

  // signals on Interest received
  signal::Signal<LinkService, Interest> afterReceiveInterest;

  // signals on Data received
  signal::Signal<LinkService, Data> afterReceiveData;


  // signals on Nack received
  signal::Signal<LinkService, lp::Nack> afterReceiveNack;

  // signals on Interest dropped by reliability system for exceeding allowed number of retx
  signal::Signal<LinkService, Interest> onDroppedInterest;

  // signals on Interest sent
  signal::Signal<LinkService, Interest> afterSendInterest;

  // signals on Data sent
  signal::Signal<LinkService, Data> afterSendData;

  // signals on Nack sent
  signal::Signal<LinkService, lp::Nack> afterSendNack;
/********************************signals****************************************/

public: // lower interface to be invoked by Transport

/*******************lower interface to be invoked by Transport**********************/

  // performs LinkService specific operations to receive a lower-layer packet
  void receivePacket(Transport::Packet&& packet);
/*******************lower interface to be invoked by Transport**********************/



/*******************upper interface to be invoked in subclass**********************/
protected: // upper interface to be invoked in subclass (receive path termination)

  // delivers received Interest to forwarding
  void receiveInterest(const Interest& interest);

  // delivers received Data to forwarding
  void receiveData(const Data& data);

  // delivers received Nack to forwarding
  void receiveNack(const lp::Nack& nack);
/*******************upper interface to be invoked in subclass**********************/


/*******************lower interface to be invoked in subclass**********************/
protected: // lower interface to be invoked in subclass (send path termination)

  // sends a lower-layer packet via Transport
  void sendPacket(Transport::Packet&& packet);

protected:
  void notifyDroppedInterest(const Interest& packet);
/*******************lower interface to be invoked in subclass**********************/


/***********************************virtual************************************/
private: // upper interface to be overridden in subclass (send path entrypoint)


  // performs LinkService specific operations to send an Interest
  virtual void doSendInterest(const Interest& interest) = 0;


  //performs LinkService specific operations to send a Data
  virtual void doSendData(const Data& data) = 0;


  // performs LinkService specific operations to send a Nack
  virtual void doSendNack(const lp::Nack& nack) = 0;

private:
  // lower interface to be overridden in subclass
  virtual void doReceivePacket(Transport::Packet&& packet) = 0;

/***********************************virtual************************************/
private:
  Face* m_face;
  Transport* m_transport;
};

/*******************************************************************************/
inline const Face* LinkService::getFace() const{
  return m_face;
}

inline const Transport* LinkService::getTransport() const{
  return m_transport;
}

inline Transport* LinkService::getTransport(){
  return m_transport;
}

//
inline const LinkService::Counters& LinkService::getCounters() const{
  return *this;
}


inline void LinkService::receivePacket(Transport::Packet&& packet){
  doReceivePacket(std::move(packet));
}


inline void LinkService::sendPacket(Transport::Packet&& packet){
  m_transport->send(std::move(packet));
}

std::ostream& operator<<(std::ostream& os, const FaceLogHelper<LinkService>& flh);

template<typename T>
typename std::enable_if<std::is_base_of<LinkService, T>::value &&
                        !std::is_same<LinkService, T>::value, std::ostream&>::type
operator<<(std::ostream& os, const FaceLogHelper<T>& flh)
{
  return os << FaceLogHelper<LinkService>(flh.obj);
}
/********************************************************************************/

} // namespace face
} // namespace nfd
```

/NFD/daemon/face/link-service.cpp
```cpp
namespace nfd {
namespace face {

NFD_LOG_INIT(LinkService);

LinkService::LinkService()
  : m_face(nullptr)
  , m_transport(nullptr){
}

LinkService::~LinkService(){
}


void LinkService::setFaceAndTransport(Face& face, Transport& transport){
  BOOST_ASSERT(m_face == nullptr);
  BOOST_ASSERT(m_transport == nullptr);

  m_face = &face;
  m_transport = &transport;
}


void LinkService::sendInterest(const Interest& interest){
  BOOST_ASSERT(m_transport != nullptr);
  NFD_LOG_FACE_TRACE(__func__);

  // 调用operator++(prefix++)
  ++this->nOutInterests;

  doSendInterest(interest);

  afterSendInterest(interest);
}

void LinkService::sendData(const Data& data){
  BOOST_ASSERT(m_transport != nullptr);
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nOutData;

  doSendData(data);

  afterSendData(data);
}

void LinkService::sendNack(const ndn::lp::Nack& nack){
  BOOST_ASSERT(m_transport != nullptr);
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nOutNacks;

  doSendNack(nack);

  afterSendNack(nack);
}

void LinkService::receiveInterest(const Interest& interest){
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nInInterests;

  afterReceiveInterest(interest);
}

void LinkService::receiveData(const Data& data){
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nInData;

  afterReceiveData(data);
}

void LinkService::receiveNack(const ndn::lp::Nack& nack){
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nInNacks;

  afterReceiveNack(nack);
}

void LinkService::notifyDroppedInterest(const Interest& interest){
  ++this->nDroppedInterests;
  onDroppedInterest(interest);
}

std::ostream&
operator<<(std::ostream& os, const FaceLogHelper<LinkService>& flh)
{
  
  const Face* face = flh.obj.getFace();
  if (face == nullptr) {
    os << "[id=0,local=unknown,remote=unknown] ";
  }
  else {
    os << "[id=" << face->getId() << ",local=" << face->getLocalUri()
       << ",remote=" << face->getRemoteUri() << "] ";
  }
  return os;
}

} // namespace face
} // namespace nfd
```
