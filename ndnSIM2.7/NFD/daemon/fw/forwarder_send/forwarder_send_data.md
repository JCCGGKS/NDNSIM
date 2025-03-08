# 发送数据包的流程

## onOutgoingData

src/ndnSIM/NFD/daemon/fw/forwarder.cpp

```cpp
void Forwarder::onOutgoingData(const Data& data, Face& outFace)
{
  if (outFace.getId() == face::INVALID_FACEID) {
    NFD_LOG_WARN("onOutgoingData face=invalid data=" << data.getName());
    return;
  }
  NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() << " data=" << data.getName());

 
  // localhost scope control
  /*
  [1]true:
  face 是非本地的，LOCALHOST是数据data名称的前缀----违反本地请求
  outFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL 
  scope_prefix::LOCALHOST.isPrefixOf(data.getName())
  
  [2]false:
  face 是本地的，LOCALHOST是数据data名称的前缀----不违反本地请求
  outFace.getScope() != ndn::nfd::FACE_SCOPE_NON_LOCAL 
  scope_prefix::LOCALHOST.isPrefixOf(data.getName())
  
  [3]false:
  face 是非本地的，LOCALHOST不是数据data名称的前缀----不违反本地请求
  outFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL 
  !scope_prefix::LOCALHOST.isPrefixOf(data.getName())
  
  [4]false:
  face 是本地的，LOCALHOST不是数据data名称的前缀----不违反本地请求
  outFace.getScope() != ndn::nfd::FACE_SCOPE_NON_LOCAL 
  !scope_prefix::LOCALHOST.isPrefixOf(data.getName())
  */
  bool isViolatingLocalhost = outFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() <<
                  " data=" << data.getName() << " violates /localhost");
    // (drop)
    return;
  }


  // send Data
  outFace.sendData(data);
  // 增加发送出去的数据包个数
  ++m_counters.nOutData;
}
```

## sendData

src/ndnSIM/NFD/daemon/face/face.hpp

```cpp
inline void
Face::sendData(const Data& data)
{
  m_service->sendData(data);
}
```

