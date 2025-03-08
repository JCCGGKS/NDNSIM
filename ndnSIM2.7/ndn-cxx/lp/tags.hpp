SimpleTag的声明在ndn-cxx/tag.hpp
```cpp
namespace ndn {
namespace lp {

/** \class IncomingFaceIdTag
 *  \brief a packet tag for IncomingFaceId field
 *
 *  This tag can be attached to Interest, Data, Nack.
 */
typedef SimpleTag<uint64_t, 10> IncomingFaceIdTag;

/** \class NextHopFaceIdTag
 *  \brief a packet tag for NextHopFaceId field
 *
 *  This tag can be attached to Interest.
 */
typedef SimpleTag<uint64_t, 11> NextHopFaceIdTag;

/** \class CachePolicyTag
 *  \brief a packet tag for CachePolicy field
 *
 *  This tag can be attached to Data.
 */
typedef SimpleTag<CachePolicy, 12> CachePolicyTag;

/** \class CongestionMarkTag
 *  \brief a packet tag for CongestionMark field
 *
 *  This tag can be attached to Interest, Data, Nack.
 */
typedef SimpleTag<uint64_t, 13> CongestionMarkTag;

/** \class NonDiscoveryTag
 *  \brief a packet tag for NonDiscovery field
 *
 *  This tag can be attached to Interest.
 */
typedef SimpleTag<EmptyValue, 14> NonDiscoveryTag;

/** \class PrefixAnnouncementTag
 *  \brief a packet tag for PrefixAnnouncement field
 *
 *  This tag can be attached to Data.
 */
typedef SimpleTag<PrefixAnnouncementHeader, 15> PrefixAnnouncementTag;

/** HopCountTag
 *  a packet tag for HopCount field
 *
 * This tag can be attached to Interest, Data, Nack.
 */
typedef SimpleTag<uint64_t, 0x60000000> HopCountTag;


} // namespace lp
} // namespace ndn
```
