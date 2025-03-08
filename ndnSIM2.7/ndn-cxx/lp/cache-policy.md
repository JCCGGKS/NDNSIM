ndn-cxx/lp/cache-policy.hpp
```cpp
namespace ndn {
namespace lp {

// 表示应用于数据包的缓存策略
enum class CachePolicyType {
  NONE = 0,
  NO_CACHE = 1
};

std::ostream& operator<<(std::ostream& os, CachePolicyType policy);

// 表示CachePolicy头字段
class CachePolicy
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    using ndn::tlv::Error::Error;
  };

  CachePolicy();

  explicit CachePolicy(const Block& block);

  // prepend CachePolicy to encoder
  // getPolicy() != CachePolicyType::NONE
  template<encoding::Tag TAG>
  size_t wireEncode(EncodingImpl<TAG>& encoder) const;


  // encode CachePolicy into wire format
  const Block& wireEncode() const;

  // get CachePolicyType from wire format
  void wireDecode(const Block& wire);

public: // policy type

  // get policy type code
  // 如果policy type没有设置或者不知道policy type code返回CachePolicyType::NONE
  CachePolicyType getPolicy() const;


  // set policy type code
  // 如果参数policy是CachePolicyType::NONE表明清除policy
  CachePolicy& setPolicy(CachePolicyType policy);

private:
  CachePolicyType m_policy;
  mutable Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(CachePolicy);

}
} // namespace ndn
```

ndn-cxx/lp/cache-policy.cpp
```cpp
namespace ndn {
namespace lp {

std::ostream& operator<<(std::ostream& os, CachePolicyType policy)
{
  switch (policy) {
  case CachePolicyType::NO_CACHE:
    os << "NoCache";
    break;
  default:
    os << "None";
    break;
  }

  return os;
}

CachePolicy::CachePolicy()
  : m_policy(CachePolicyType::NONE){
}

CachePolicy::CachePolicy(const Block& block){
  wireDecode(block);
}

template<encoding::Tag TAG>
size_t CachePolicy::wireEncode(EncodingImpl<TAG>& encoder) const{
  if (m_policy == CachePolicyType::NONE) {
    BOOST_THROW_EXCEPTION(Error("CachePolicyType must be set"));
  }

  size_t length = 0;
  length += prependNonNegativeIntegerBlock(encoder, tlv::CachePolicyType, static_cast<uint32_t>(m_policy));
  length += encoder.prependVarNumber(length);
  length += encoder.prependVarNumber(tlv::CachePolicy);
  return length;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(CachePolicy);

const Block& CachePolicy::wireEncode() const{
  if (m_policy == CachePolicyType::NONE) {
    BOOST_THROW_EXCEPTION(Error("CachePolicyType must be set"));
  }

  if (m_wire.hasWire()) {
    return m_wire;
  }

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void CachePolicy::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::CachePolicy) {
    BOOST_THROW_EXCEPTION(Error("expecting CachePolicy block"));
  }

  m_wire = wire;
  m_wire.parse();

  Block::element_const_iterator it = m_wire.elements_begin();
  if (it != m_wire.elements_end() && it->type() == tlv::CachePolicyType) {
    m_policy = static_cast<CachePolicyType>(readNonNegativeInteger(*it));
    if (this->getPolicy() == CachePolicyType::NONE) {
      BOOST_THROW_EXCEPTION(Error("unknown CachePolicyType"));
    }
  }
  else {
    BOOST_THROW_EXCEPTION(Error("expecting CachePolicyType block"));
  }
}

CachePolicyType CachePolicy::getPolicy() const{
  switch (m_policy) {
  case CachePolicyType::NO_CACHE:
    return m_policy;
  default:
    return CachePolicyType::NONE;
  }
}

CachePolicy& CachePolicy::setPolicy(CachePolicyType policy){
  m_policy = policy;
  m_wire.reset();
  return *this;
}

} // namespace lp
} // namespace ndn
```
