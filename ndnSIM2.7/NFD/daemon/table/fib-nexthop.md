/NFD/daemon/table/fib-nexthop.hpp
```cpp
namespace nfd {
namespace fib {

/** \class NextHop
 *  \brief represents a nexthop record in FIB entry
 */
class NextHop
{
public:
  explicit NextHop(Face& face, uint64_t endpointId);

  Face& getFace() const{
    return *m_face;
  }

  uint64_t getEndpointId() const{
    return m_endpointId;
  }

  uint64_t getCost() const{
    return m_cost;
  }

  void setCost(uint64_t cost){
    m_cost = cost;
  }

private:
  Face* m_face;
  uint64_t m_endpointId;
  uint64_t m_cost;
};

} // namespace fib
} // namespace nfd
```

/NFD/daemon/table/fib-nexthop.cpp
```cpp
namespace nfd {
namespace fib {

NextHop::NextHop(Face& face, uint64_t endpointId)
  : m_face(&face)
  , m_endpointId(endpointId)
  , m_cost(0){
}

} // namespace fib
} // namespace nfd
```
