# 发送兴趣包的流程

## onOutgoingInterest

src/ndnSIM/NFD/daemon/fw/forwarder.cpp

```cpp
void
Forwarder::onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry, Face& outFace, const Interest& interest)
{
  NFD_LOG_DEBUG("onOutgoingInterest face=" << outFace.getId() <<
                " interest=" << pitEntry->getName());

  // insert or update out-record
  pitEntry->insertOrUpdateOutRecord(outFace, interest);

  // send Interest
  outFace.sendInterest(interest);
  // 增加传出的兴趣包个数
  ++m_counters.nOutInterests;
}
```

## sendInterest

src/ndnSIM/NFD/daemon/face/face.hpp

```cpp
inline void
Face::sendInterest(const Interest& interest)
{
  m_service->sendInterest(interest);
}
```

