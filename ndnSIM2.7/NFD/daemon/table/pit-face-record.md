/NFD/daemon/table/pit-face-record.hpp
```cpp
namespace nfd {
namespace pit {

/******************************************************************************/

// contains information about an Interest on an incoming or outgoing face
// This is an implementation detail to extract common functionality of InRecord and OutRecord
// 包含输入或者输出Face中兴趣包的相关信息
// 这是一个实现细节，用于提取InRecord和OutRecord的公共功能
class FaceRecord : public StrategyInfoHost
{
public:
  // 显式构造函数
  explicit FaceRecord(Face& face);

  Face& getFace() const;

  uint32_t getLastNonce() const;

  time::steady_clock::TimePoint getLastRenewed() const;

  // 给出此记录到期的时间点
  // return getLastRenewed() + InterestLifetime
  time::steady_clock::TimePoint getExpiry() const;


  // updates lastNonce, lastRenewed, expiry fields
  void update(const Interest& interest);

private:
  Face& m_face;
  uint32_t m_lastNonce;
  time::steady_clock::TimePoint m_lastRenewed;
  time::steady_clock::TimePoint m_expiry;
};

/**********************************************************************/
inline Face& FaceRecord::getFace() const{
  return m_face;
}

inline uint32_t FaceRecord::getLastNonce() const{
  return m_lastNonce;
}

inline time::steady_clock::TimePoint FaceRecord::getLastRenewed() const{
  return m_lastRenewed;
}

inline time::steady_clock::TimePoint FaceRecord::getExpiry() const{
  return m_expiry;
}
/***************************************************************************/

} // namespace pit
} // namespace nfd
```

/NFD/daemon/table/pit-face-record.cpp
```cpp
namespace nfd {
namespace pit {

FaceRecord::FaceRecord(Face& face)
  : m_face(face)
  , m_lastNonce(0)
  , m_lastRenewed(time::steady_clock::TimePoint::min())
  , m_expiry(time::steady_clock::TimePoint::min()){
}

// 更新lastNonce, lastRenewed, expiry fields
void FaceRecord::update(const Interest& interest){
  m_lastNonce = interest.getNonce();
  m_lastRenewed = time::steady_clock::now();

  time::milliseconds lifetime = interest.getInterestLifetime();
  if (lifetime < time::milliseconds::zero()) {
    // 校正lifetime字段的值
    // const time::milliseconds DEFAULT_INTEREST_LIFETIME = 4_s;
    lifetime = ndn::DEFAULT_INTEREST_LIFETIME;
  }
  // 绝对的过期时间，这个是兴趣包的过期时间，和数据包的过期时间freshnessperiod不一样
  m_expiry = m_lastRenewed + lifetime;
}


} // namespace pit
} // namespace nfd
```
