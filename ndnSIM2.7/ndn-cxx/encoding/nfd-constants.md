ndn-cxx/encoding/nfd-constants.hpp
```cpp
namespace ndn {
namespace nfd {

const uint64_t INVALID_FACE_ID = 0;
/**
enum FaceScope: uint8_t{  
};
指定enum的大小，这样enum的潜在成员类型就是uint8_t
枚举成员的值不能超出uint8_t能表示的范围
*/
// ingroup management
enum FaceScope : uint8_t {
  FACE_SCOPE_NONE      = std::numeric_limits<uint8_t>::max(),
  FACE_SCOPE_NON_LOCAL = 0, ///< face is non-local
  FACE_SCOPE_LOCAL     = 1, ///< face is local
};

std::ostream& operator<<(std::ostream& os, FaceScope faceScope);


// ingroup management
enum FacePersistency : uint8_t {
  FACE_PERSISTENCY_NONE       = std::numeric_limits<uint8_t>::max(),
  FACE_PERSISTENCY_PERSISTENT = 0, ///< face is persistent
  FACE_PERSISTENCY_ON_DEMAND  = 1, ///< face is on-demand
  FACE_PERSISTENCY_PERMANENT  = 2, ///< face is permanent
};

std::ostream& operator<<(std::ostream& os, FacePersistency facePersistency);


// ingroup management
enum LinkType : uint8_t {
  LINK_TYPE_NONE           = std::numeric_limits<uint8_t>::max(),
  LINK_TYPE_POINT_TO_POINT = 0, ///< link is point-to-point
  LINK_TYPE_MULTI_ACCESS   = 1, ///< link is multi-access
  LINK_TYPE_AD_HOC         = 2, ///< link is ad hoc
};

std::ostream& operator<<(std::ostream& os, LinkType linkType);


// ingroup management
enum FaceFlagBit {
  BIT_LOCAL_FIELDS_ENABLED = 0, ///< whether local fields are enabled on a face
  BIT_LP_RELIABILITY_ENABLED = 1, ///< whether the link reliability feature is enabled on a face
  BIT_CONGESTION_MARKING_ENABLED = 2, ///< whether congestion detection and marking is enabled on a face
};


// ingroup management
enum FaceEventKind : uint8_t {
  FACE_EVENT_NONE      = 0,
  FACE_EVENT_CREATED   = 1, ///< face was created
  FACE_EVENT_DESTROYED = 2, ///< face was destroyed
  FACE_EVENT_UP        = 3, ///< face went UP (from DOWN state)
  FACE_EVENT_DOWN      = 4, ///< face went DOWN (from UP state)
};


std::ostream& operator<<(std::ostream& os, FaceEventKind faceEventKind);

/** \ingroup management
 *  \brief CS enablement flags
 *  \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
 */
enum CsFlagBit {
  BIT_CS_ENABLE_ADMIT = 0, ///< enables the CS to admit new Data
  BIT_CS_ENABLE_SERVE = 1, ///< enables the CS to satisfy Interests using cached Data
};


// ingroup management
enum RouteOrigin : uint16_t {
  ROUTE_ORIGIN_NONE          = std::numeric_limits<uint16_t>::max(),
  ROUTE_ORIGIN_APP           = 0,
  ROUTE_ORIGIN_AUTOREG       = 64,
  ROUTE_ORIGIN_CLIENT        = 65,
  ROUTE_ORIGIN_AUTOCONF      = 66,
  ROUTE_ORIGIN_NLSR          = 128,
  ROUTE_ORIGIN_PREFIXANN     = 129,
  ROUTE_ORIGIN_STATIC        = 255,
};

/** \brief extract RouteOrigin from stream
 *  \post if the first token in contains a valid RouteOrigin as string or numeric value, it is
 *        extracted into routeOrigin ; otherwise, routeOrigin is set to ROUTE_ORIGIN_NONE ,
 *        and failbit is set on  is
 */
std::istream& operator>>(std::istream& is, RouteOrigin& routeOrigin);


std::ostream& operator<<(std::ostream& os, RouteOrigin routeOrigin);


// ingroup management
enum RouteFlags : uint64_t {
  ROUTE_FLAGS_NONE         = 0,
  ROUTE_FLAG_CHILD_INHERIT = 1,
  ROUTE_FLAG_CAPTURE       = 2,
};

std::ostream& operator<<(std::ostream& os, RouteFlags routeFlags);

} // namespace nfd
} // namespace ndn
```
