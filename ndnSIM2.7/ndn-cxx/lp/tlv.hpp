ndn-cxx/lp/tlv.hpp
```cpp
namespace ndn {
namespace lp {
namespace tlv {

// TLV-TYPE
enum {
  LpPacket = 100,
  Fragment = 80,
  Sequence = 81,
  FragIndex = 82,
  FragCount = 83,
  HopCountTag = 84, 
  Nack = 800,
  NackReason = 801,
  NextHopFaceId = 816,
  IncomingFaceId = 817,
  CachePolicy = 820,
  CachePolicyType = 821,
  CongestionMark = 832,
  Ack = 836,
  TxSequence = 840,
  NonDiscovery = 844,
  PrefixAnnouncement = 848,
};

enum {
  /**
   * \brief lower bound of 1-octet header field
   */
  HEADER1_MIN = 81,

  /**
   * \brief upper bound of 1-octet header field
   */
  HEADER1_MAX = 99,

  /**
   * \brief lower bound of 3-octet header field
   */
  HEADER3_MIN = 800,

  /**
   * \brief upper bound of 3-octet header field
   */
  HEADER3_MAX = 959
};

} // namespace tlv
} // namespace lp
} // namespace ndn
```
