/ndn-cxx/interest.hpp
```cpp
namespace ndn {

// 声明Data
class Data;


// const unspecified_duration_type DEFAULT_INTEREST_LIFETIME
// InterestLifetime的默认值：4s
const time::milliseconds DEFAULT_INTEREST_LIFETIME = 4_s;

// Represents an Interest packet.
class Interest : public PacketBase, public std::enable_shared_from_this<Interest>
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  /** Construct an Interest with given name and lifetime.
   *  @throw std::invalid_argument : lifetime is negative
   *  @warning In certain contexts that use `Interest::shared_from_this()`, Interest must be created
   *           using `make_shared`. Otherwise, `shared_from_this()` will trigger undefined behavior.
   */
  // 显式构造函数
  // 用name和lifetime构造一个interest
  // lifetime如果是负值抛出std::invalid_argument
  // 在使用' Interest::shared_from_this() '的特定上下文中，必须使用' make_shared '创建Interest。否则，' shared_from_this() '将触发未定义的行为。
  explicit Interest(const Name& name = Name(), time::milliseconds lifetime = DEFAULT_INTEREST_LIFETIME);


  /** Construct an Interest by decoding from  wire.
   *  @warning In certain contexts that use `Interest::shared_from_this()`, Interest must be created
   *           using `make_shared`. Otherwise, `shared_from_this()` will trigger undefined behavior.
   */
  // 通过解码wire构造interest
  explicit Interest(const Block& wire);


  // Prepend wire encoding to encoder.
  template<encoding::Tag TAG>
  size_t wireEncode(EncodingImpl<TAG>& encoder) const;

  // Encode to a Block.
  // 如果parameteres元素存在，则编码为NDN包格式v0.3。在这种情况下，Selectors没有被编码。
  // 否则编码为NDN报文格式v0.2。
  const Block& wireEncode() const;

  // Decode from wire in NDN Packet Format v0.2 or v0.3.
  void wireDecode(const Block& wire);

  // Check if this instance has cached wire encoding.
  bool hasWire() const{
    return m_wire.hasWire();
  }

  /** Return a URI-like string that represents the Interest.
   *
   *  The string starts with `getName().toUri()`.
   *  If the Interest contains selectors, they are included as a query string.
   *  Example: "/test/name?ndn.MustBeFresh=1"
   */
  std::string toUri() const;

public: // matching

  // Check if Interest, including selectors, matches the given name
  // name : The name to be matched. If this is a Data name, it shall contain the implicit digest component
  // 检查兴趣包是否包含selectors，并且匹配被给的参数name
  // name：如果这是一个Data name，它将包含隐式摘要
  bool matchesName(const Name& name) const;

  /** Check if Interest can be satisfied by data.
   *
   *  This method considers Name, MinSuffixComponents, MaxSuffixComponents,
   *  PublisherPublicKeyLocator, and Exclude.
   *  This method does not consider ChildSelector and MustBeFresh.
   */
  /**  判断兴趣包是否可以被传入的参数data满足
  * 该方法考虑Name、MinSuffixComponents、MaxSuffixComponents、PublisherPublicKeyLocator和Exclude。
  * 这个方法不考虑ChildSelector和MustBeFresh。
  */
  bool matchesData(const Data& data) const;


  /** Check if Interest matches other interest
   *
   *  Interest matches other if both have the same name, selectors, and link.  Other fields
   *  (e.g., Nonce) may be different.
   *
   *  @todo Implement distinguishing Interests by forwarding hint. The current implementation
   *        checks only name+selectors (Issue #3162).
   */
  // 检查兴趣包是否可以匹配其它兴趣包
  // 如果两者具有相同的名称、选择器和链接，则Interest匹配other。其他元素(例如，Nonce)可能会有所不同。
  // 通过转发提示实现兴趣区分。当前的实现只检查名称+选择器(问题#3162)。
  bool matchesInterest(const Interest& other) const;

public: // element access

  const Name& getName() const{
    return m_name;
  }

  Interest& setName(const Name& name){
    m_name = name;
    m_wire.reset();
    return *this;
  }

  /** Declare the default CanBePrefix setting of the application.
   *
   *  As part of transitioning to NDN Packet Format v0.3, the default setting for CanBePrefix
   *  will be changed from "true" to "false". Application developers are advised to review all
   *  Interests expressed by their application and decide what CanBePrefix setting is appropriate
   *  for each Interest, to avoid breaking changes when the transition occurs. Application may
   *  either set CanBePrefix on a per-Interest basis, or declare a default CanBePrefix setting for
   *  all Interests expressed by the application using this function. If an application neither
   *  declares a default nor sets CanBePrefix on every Interest, Interest::wireEncode will print a
   *  one-time warning message.
   *
   *  @note This function should not be used in libraries or in ndn-cxx unit tests.
   *  @sa https://redmine.named-data.net/projects/nfd/wiki/Packet03Transition
   */
  /** 声明应用程序的默认CanBePrefix设置。
  * 作为向NDN数据包格式v0.3过渡的一部分，CanBePrefix的默认设置将从“true”变更为“false”。
  * 建议应用程序开发人员审查其应用所表达的所有兴趣，并确定每个兴趣的适当CanBePrefix设置，以避免在过渡发生时出现破坏性更改。应用程序可以基于每个兴趣设置CanBePrefix，
  * 或者使用此函数为应用程序所表达的所有兴趣声明默认的CanBePrefix设置。如果应用程序既未声明默认值，也未为每个兴趣设置CanBePrefix，则Interest::wireEncode 将打印一次性警告消息。
  * note : 此函数不应在库或ndn-cxx单元测试中使用。
  */
  // 静态成员函数只能访问静态成员变量
  static void setDefaultCanBePrefix(bool canBePrefix){
    s_defaultCanBePrefix = canBePrefix;
  }


/******************************canBePrefix*************************************/
  /** Check whether the CanBePrefix element is present.
   *
   *  This is a getter for the CanBePrefix element as defined in NDN Packet Format v0.3.
   *  In this implementation, it is mapped to the closest v0.2 semantics:
   *  MaxSuffixComponents=1 means CanBePrefix is absent.
   */
  /**  检查是否存在CanBePrefix元素。
  * 这是CanBePrefix元素的getter，在NDN Packet Format v0.3中定义。
  * 在这个实现中，它被映射到最接近的v0.2语义:MaxSuffixComponents=1意味着CanBePrefix不存在。
  */
  bool getCanBePrefix() const{
    // MaxSuffixComponents=1意味着CanBePrefix不存在
    return m_selectors.getMaxSuffixComponents() != 1;
  }

  /** Add or remove CanBePrefix element.
   *  @param canBePrefix : whether CanBePrefix element should be present.
   *
   *  This is a setter for the CanBePrefix element as defined in NDN Packet Format v0.3.
   *  In this implementation, it is mapped to the closest v0.2 semantics:
   *  MaxSuffixComponents=1 means CanBePrefix is absent.
   */
  /**  添加或者移除CanBePrefix元素
  * 这是CanBePrefix元素的getter，在NDN Packet Format v0.3中定义。
  * 在这个实现中，它被映射到最接近的v0.2语义:MaxSuffixComponents=1意味着CanBePrefix不存在。
  */
  Interest& setCanBePrefix(bool canBePrefix){
    // MaxSuffixComponents=1意味着CanBePrefix不存在
    m_selectors.setMaxSuffixComponents(canBePrefix ? -1 : 1);
    m_wire.reset();
    m_isCanBePrefixSet = true;
    return *this;
  }
/******************************canBePrefix*************************************/


/******************************MustBeFresh*************************************/
  /** Check whether the MustBeFresh element is present.
   *
   *  This is a getter for the MustBeFresh element as defined in NDN Packet Format v0.3.
   *  In this implementation, it is mapped to the closest v0.2 semantics and appears as
   *  MustBeFresh element under Selectors.
   */
  /** 检查MustBeFresh元素是否存在
  * 这是在NDN Packet Format v0.3中定义的MustBeFresh元素的getter。
  * 在这个实现中，它被映射到最接近的v0.2语义，并在selector下显示为MustBeFresh元素。
  */
  bool getMustBeFresh() const{
    return m_selectors.getMustBeFresh();
  }

  /** Add or remove MustBeFresh element.
   *  @param mustBeFresh ： whether MustBeFresh element should be present.
   *
   *  This is a setter for the MustBeFresh element as defined in NDN Packet Format v0.3.
   *  In this implementation, it is mapped to the closest v0.2 semantics and appears as
   *  MustBeFresh element under Selectors.
   */
  /** 添加或者移除MustBeFresh元素
  * 这是在NDN Packet Format v0.3中定义的MustBeFresh元素的setter。
  * 在这个实现中，它被映射到最接近的v0.2语义，并显示为Selector下的MustBeFresh元素。
  */
  Interest& setMustBeFresh(bool mustBeFresh){
    m_selectors.setMustBeFresh(mustBeFresh);
    m_wire.reset();
    return *this;
  }
/******************************MustBeFresh*************************************/

/******************************ForwardingHint**********************************/
  const DelegationList& getForwardingHint() const{
    return m_forwardingHint;
  }

  Interest& setForwardingHint(const DelegationList& value);

  /** Modify ForwardingHint in-place.
   *  @tparam Modifier ： a unary function that accepts DelegationList&
   *
   *  This is equivalent to, but more efficient (avoids copying) than:
   *  @code
   *  auto fh = interest.getForwardingHint();
   *  modifier(fh);
   *  interest.setForwardingHint(fh);
   *  @endcode
   */
  /**  就地修改ForwardingHint
  * Modifier : 参数为DelegationList&的一元函数
  * 等效于，但是更高效(不用复制)比：
  *  @code
  *  auto fh = interest.getForwardingHint();
  *  modifier(fh);
  *  interest.setForwardingHint(fh);
  *  @endcode
  */
  template<typename Modifier>
  Interest& modifyForwardingHint(const Modifier& modifier){
    modifier(m_forwardingHint);
    m_wire.reset();
    return *this;
  }
/******************************ForwardingHint**********************************/


/************************************Nonce*************************************/

  // Check if the Nonce element is present.
  // 检查Nonce元素是否存在
  bool hasNonce() const{
    return static_cast<bool>(m_nonce);
  }

  /** Get nonce value.
   *  If nonce was not present, it is added and assigned a random value.
   */
  // 获取nonce元素的值；如果nonce元素不存在，被添加并且被赋值一个随机的数值
  uint32_t getNonce() const;

  // Set nonce value.
  Interest& setNonce(uint32_t nonce);

  /** Change nonce value.
   *  If the Nonce element is present, the new nonce value will differ from the old value.
   *  If the Nonce element is not present, this method does nothing.
   */
  // 如果存在Nonce元素，则新的Nonce值将不同于旧值。
  // 如果Nonce元素不存在，则此方法不执行任何操作。
  void refreshNonce();
/************************************Nonce**********************************************/

/************************************InterestLifetime***********************************/
  time::milliseconds getInterestLifetime() const{
    return m_interestLifetime;
  }

  /** Set Interest's lifetime
   *  @throw std::invalid_argument lifetime is negative
   */
  // 如果传入的参数lifetime是负值，抛出std::invalid_argument
  Interest& setInterestLifetime(time::milliseconds lifetime);
/************************************InterestLifetime***********************************/


/*************************************parameters*************************************/
  bool hasParameters() const{
    // 调用Block::empty();
    return !m_parameters.empty();
  }

  const Block& getParameters() const{
    return m_parameters;
  }

  /** Set parameters from a Block
   *  If the block's TLV-TYPE is Parameters, it will be used directly as this Interest's Parameters element.
   *  If the block's TLV-TYPE is not Parameters, it will be nested into a Parameters element.
   *  @return a reference to this Interest
   */
  // 如果块的TLV-TYPE是Parameters，它将被直接用作Interest的Parameters元素。
  // 如果块的TLV-TYPE不是Parameters，它将被嵌套到Parameters元素中。
  Interest& setParameters(const Block& parameters);

  /** Copy parameters from raw buffer
   *  @param buffer : pointer to the first octet of parameters
   *  @param bufferSize : size of the raw buffer
   *  @return a reference to this Interest
   */
  Interest& setParameters(const uint8_t* buffer, size_t bufferSize);


  /** Set parameters from a wire buffer
   *  @param buffer : containing the Interest parameters
   *  @return a reference to this Interest
   */
  Interest& setParameters(ConstBufferPtr buffer);


  /** @brief Remove the Parameters element from this Interest
   *  @post hasParameters() == false
   */
  Interest& unsetParameters();
/*************************************parameters*************************************/


/**************************************deprecated****************************/
/**deprecated
* C++14标准引入的新属性，用于标记某个函数、类、枚举或者变量等实体为已弃用
* 意味着这些实体仍然可以被使用但是不推荐，因为可能在将来的版本中被移除或者被更新
* 使用了deprecated标记的实体，编译器会发出警告，这有助于开发者维护代码。
*/
public: // Selectors (deprecated)
  /** @brief Check if Interest has any selector present.
   */
  [[deprecated]]
  bool
  hasSelectors() const
  {
    return !m_selectors.empty();
  }

  [[deprecated]]
  const Selectors&
  getSelectors() const
  {
    return m_selectors;
  }

  [[deprecated]]
  Interest&
  setSelectors(const Selectors& selectors)
  {
    m_selectors = selectors;
    m_wire.reset();
    return *this;
  }

  [[deprecated]]
  int
  getMinSuffixComponents() const
  {
    return m_selectors.getMinSuffixComponents();
  }

  [[deprecated]]
  Interest&
  setMinSuffixComponents(int minSuffixComponents)
  {
    m_selectors.setMinSuffixComponents(minSuffixComponents);
    m_wire.reset();
    return *this;
  }

  [[deprecated]]
  int
  getMaxSuffixComponents() const
  {
    return m_selectors.getMaxSuffixComponents();
  }

  [[deprecated]]
  Interest&
  setMaxSuffixComponents(int maxSuffixComponents)
  {
    m_selectors.setMaxSuffixComponents(maxSuffixComponents);
    m_wire.reset();
    return *this;
  }

  [[deprecated]]
  const KeyLocator&
  getPublisherPublicKeyLocator() const
  {
    return m_selectors.getPublisherPublicKeyLocator();
  }

  [[deprecated]]
  Interest&
  setPublisherPublicKeyLocator(const KeyLocator& keyLocator)
  {
    m_selectors.setPublisherPublicKeyLocator(keyLocator);
    m_wire.reset();
    return *this;
  }

  [[deprecated]]
  const Exclude&
  getExclude() const
  {
    return m_selectors.getExclude();
  }

  [[deprecated]]
  Interest&
  setExclude(const Exclude& exclude)
  {
    m_selectors.setExclude(exclude);
    m_wire.reset();
    return *this;
  }

  [[deprecated]]
  int
  getChildSelector() const
  {
    return m_selectors.getChildSelector();
  }

  [[deprecated]]
  Interest&
  setChildSelector(int childSelector)
  {
    m_selectors.setChildSelector(childSelector);
    m_wire.reset();
    return *this;
  }
/**************************************deprecated****************************/
private:
/************************encode - decode ***************************************/

  // Prepend wire encoding to encoder in NDN Packet Format v0.2.
  // 按照v0.2的包格式规范对encoder进行编码
  template<encoding::Tag TAG>
  size_t encode02(EncodingImpl<TAG>& encoder) const;


  // Prepend wire encoding to encoder in NDN Packet Format v0.3.
  // 按照v0.3的包格式规范对encoder进行编码
  template<encoding::Tag TAG>
  size_t encode03(EncodingImpl<TAG>& encoder) const;


  /** Decode  m_wire as NDN Packet Format v0.2.
   *  @retval true ： decoding successful.
   *  @retval false ： decoding failed due to structural error.
   *  @throw tlv::Error ： decoding error within a sub-element.
   */
  // 按照v0.2包格式规范对m_wire进行解码
  // 解码成功返回true；由于结构错误解码失败返回false
  // sub-element解码失败抛出tlv::error
  bool decode02();

  // Decode  m_wire as NDN Packet Format v0.3.
  // 如果解码错误抛出tlv::Error.
  void decode03();
/************************encode - decode ***************************************/

#ifdef NDN_CXX_HAVE_TESTS
public:

  // If true, not setting CanBePrefix results in an error in wireEncode().
  static bool s_errorIfCanBePrefixUnset;
#endif // NDN_CXX_HAVE_TESTS

private:
  // 声明成员静态变量，需要在类外定义
  static boost::logic::tribool s_defaultCanBePrefix;

  Name m_name;
  Selectors m_selectors; // NDN Packet Format v0.2 only
  mutable bool m_isCanBePrefixSet;
  mutable optional<uint32_t> m_nonce;
  time::milliseconds m_interestLifetime;
  DelegationList m_forwardingHint;
  Block m_parameters; // NDN Packet Format v0.3 only

  // mutable表示该变量尽管是在const成员函数内部也是可以被修改的
  mutable Block m_wire;

  // 声明友元函数
  friend bool operator==(const Interest& lhs, const Interest& rhs);
};

/*************************************************************************/
NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Interest);

std::ostream& operator<<(std::ostream& os, const Interest& interest);

bool operator==(const Interest& lhs, const Interest& rhs);

inline bool operator!=(const Interest& lhs, const Interest& rhs){
  return !(lhs == rhs);
}
/**************************************************************************/

} // namespace ndn
```

/ndn-cxx/interest.cpp
```cpp
namespace ndn {

BOOST_CONCEPT_ASSERT((boost::EqualityComparable<Interest>));
BOOST_CONCEPT_ASSERT((WireEncodable<Interest>));
BOOST_CONCEPT_ASSERT((WireEncodableWithEncodingBuffer<Interest>));
BOOST_CONCEPT_ASSERT((WireDecodable<Interest>));
static_assert(std::is_base_of<tlv::Error, Interest::Error>::value,
              "Interest::Error must inherit from tlv::Error");

#ifdef NDN_CXX_HAVE_TESTS
// 定义类外静态变量
bool Interest::s_errorIfCanBePrefixUnset = true;
#endif // NDN_CXX_HAVE_TESTS
// 类外定义静态变量
boost::logic::tribool Interest::s_defaultCanBePrefix = boost::logic::indeterminate;

Interest::Interest(const Name& name, time::milliseconds lifetime)
  : m_name(name)
  , m_isCanBePrefixSet(false)
  , m_interestLifetime(lifetime){

  // lifetime小于零，抛出异常std::invalid_argument
  if (lifetime < time::milliseconds::zero()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("InterestLifetime must be >= 0"));
  }

  if (!boost::logic::indeterminate(s_defaultCanBePrefix)) {
    setCanBePrefix(static_cast<bool>(s_defaultCanBePrefix));
  }
}

Interest::Interest(const Block& wire)
  : m_isCanBePrefixSet(true){
  wireDecode(wire);
}

// ---- encode and decode ----

template<encoding::Tag TAG>
size_t Interest::wireEncode(EncodingImpl<TAG>& encoder) const{
  // 局部静态变量
  static bool hasDefaultCanBePrefixWarning = false;

  // 元素CanBePrefix没有设置
  if (!m_isCanBePrefixSet) {
    if (!hasDefaultCanBePrefixWarning) {
      std::cerr << "WARNING: Interest.CanBePrefix will be set to 0 in the near future. "
                << "Please declare a preferred setting via Interest::setDefaultCanBePrefix.";
      hasDefaultCanBePrefixWarning = true;
    }
#ifdef NDN_CXX_HAVE_TESTS
    if (s_errorIfCanBePrefixUnset) {
      BOOST_THROW_EXCEPTION(std::logic_error("Interest.CanBePrefix is unset"));
    }
#endif // NDN_CXX_HAVE_TESTS
  }

  // 有Parameters，按照v0.3包格式规范编码
  if (hasParameters()) {
    return encode03(encoder);
  }
  else {
    // 没有Parameters，按照v0.2包格式规范编码
    return encode02(encoder);
  }
}

template<encoding::Tag TAG>
size_t Interest::encode02(EncodingImpl<TAG>& encoder) const{

  size_t totalLength = 0;

  // Encode as NDN Packet Format v0.2
  // Interest ::= INTEREST-TYPE TLV-LENGTH
  //                Name
  //                Selectors?
  //                Nonce
  //                InterestLifetime?
  //                ForwardingHint?

  // (reverse encoding)

  // ForwardingHint
  if (getForwardingHint().size() > 0) {
    totalLength += getForwardingHint().wireEncode(encoder);
  }

  // InterestLifetime
  if (getInterestLifetime() != DEFAULT_INTEREST_LIFETIME) {
    totalLength += prependNonNegativeIntegerBlock(encoder,
                                                  tlv::InterestLifetime,
                                                  getInterestLifetime().count());
  }

  // Nonce
  uint32_t nonce = getNonce(); // if nonce was unset, getNonce generates a random nonce
  totalLength += encoder.prependByteArrayBlock(tlv::Nonce, reinterpret_cast<uint8_t*>(&nonce), sizeof(nonce));

  // Selectors
  if (hasSelectors()) {
    totalLength += getSelectors().wireEncode(encoder);
  }

  // Name
  totalLength += getName().wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::Interest);
  return totalLength;
}

template<encoding::Tag TAG>
size_t Interest::encode03(EncodingImpl<TAG>& encoder) const{
  size_t totalLength = 0;

  // Encode as NDN Packet Format v0.3
  // Interest ::= INTEREST-TYPE TLV-LENGTH
  //                Name
  //                CanBePrefix?
  //                MustBeFresh?
  //                ForwardingHint?
  //                Nonce?
  //                InterestLifetime?
  //                HopLimit?
  //                Parameters?

  // (reverse encoding)

  // Parameters
  if (hasParameters()) {
    totalLength += encoder.prependBlock(getParameters());
  }

  // HopLimit: not yet supported

  // InterestLifetime
  if (getInterestLifetime() != DEFAULT_INTEREST_LIFETIME) {
    totalLength += prependNonNegativeIntegerBlock(encoder,
                                                  tlv::InterestLifetime,
                                                  getInterestLifetime().count());
  }

  // Nonce
  uint32_t nonce = getNonce(); // if nonce was unset, getNonce generates a random nonce
  totalLength += encoder.prependByteArrayBlock(tlv::Nonce, reinterpret_cast<uint8_t*>(&nonce), sizeof(nonce));

  // ForwardingHint
  if (getForwardingHint().size() > 0) {
    totalLength += getForwardingHint().wireEncode(encoder);
  }

  // MustBeFresh
  if (getMustBeFresh()) {
    totalLength += prependEmptyBlock(encoder, tlv::MustBeFresh);
  }

  // CanBePrefix
  if (getCanBePrefix()) {
    totalLength += prependEmptyBlock(encoder, tlv::CanBePrefix);
  }

  // Name
  totalLength += getName().wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::Interest);
  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(Interest);

const Block& Interest::wireEncode() const{
  if (m_wire.hasWire())
    return m_wire;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  // wireDecode是非常量成员函数，调用之前需要先用const_cast强制类型转换去除this的const属性
  const_cast<Interest*>(this)->wireDecode(buffer.block());
  return m_wire;
}

void Interest::wireDecode(const Block& wire){

  m_wire = wire;
  m_wire.parse();

  if (m_wire.type() != tlv::Interest) {
    BOOST_THROW_EXCEPTION(Error("expecting Interest element, got " + to_string(m_wire.type())));
  }

  if (!decode02()) {
    decode03();
    if (!hasNonce()) {
      // getNonce：如果没有Nonce，就插入一个新的并赋值为随机值
      setNonce(getNonce());
    }
  }

  m_isCanBePrefixSet = true; // don't trigger warning from decoded packet
}

bool Interest::decode02(){

  auto element = m_wire.elements_begin();

  // Name
  if (element != m_wire.elements_end() && element->type() == tlv::Name) {
    m_name.wireDecode(*element);
    ++element;
  }
  else {
    return false;
  }

  // Selectors?
  if (element != m_wire.elements_end() && element->type() == tlv::Selectors) {
    m_selectors.wireDecode(*element);
    ++element;
  }
  else {
    m_selectors = Selectors();
  }

  // Nonce
  if (element != m_wire.elements_end() && element->type() == tlv::Nonce) {
    uint32_t nonce = 0;
    if (element->value_size() != sizeof(nonce)) {
      BOOST_THROW_EXCEPTION(Error("Nonce element is malformed"));
    }
    std::memcpy(&nonce, element->value(), sizeof(nonce));
    m_nonce = nonce;
    ++element;
  }
  else {
    return false;
  }

  // InterestLifetime?
  if (element != m_wire.elements_end() && element->type() == tlv::InterestLifetime) {
    m_interestLifetime = time::milliseconds(readNonNegativeInteger(*element));
    ++element;
  }
  else {
    m_interestLifetime = DEFAULT_INTEREST_LIFETIME;
  }

  // ForwardingHint?
  if (element != m_wire.elements_end() && element->type() == tlv::ForwardingHint) {
    m_forwardingHint.wireDecode(*element, false);
    ++element;
  }
  else {
    m_forwardingHint = DelegationList();
  }

  return element == m_wire.elements_end();
}

void Interest::decode03(){

  // Interest ::= INTEREST-TYPE TLV-LENGTH
  //                Name
  //                CanBePrefix?
  //                MustBeFresh?
  //                ForwardingHint?
  //                Nonce?
  //                InterestLifetime?
  //                HopLimit?
  //                Parameters?

  auto element = m_wire.elements_begin();
  if (element == m_wire.elements_end() || element->type() != tlv::Name) {
    BOOST_THROW_EXCEPTION(Error("Name element is missing or out of order"));
  }
  m_name.wireDecode(*element);
  if (m_name.empty()) {
    BOOST_THROW_EXCEPTION(Error("Name has zero name components"));
  }
  int lastElement = 1; // last recognized element index, in spec order

  m_selectors = Selectors().setMaxSuffixComponents(1); // CanBePrefix=0
  m_nonce.reset();
  m_interestLifetime = DEFAULT_INTEREST_LIFETIME;
  m_forwardingHint = DelegationList();
  m_parameters = Block();

  for (++element; element != m_wire.elements_end(); ++element) {
    switch (element->type()) {
      case tlv::CanBePrefix: {
        if (lastElement >= 2) {
          BOOST_THROW_EXCEPTION(Error("CanBePrefix element is out of order"));
        }
        if (element->value_size() != 0) {
          BOOST_THROW_EXCEPTION(Error("CanBePrefix element has non-zero TLV-LENGTH"));
        }
        m_selectors.setMaxSuffixComponents(-1);
        lastElement = 2;
        break;
      }
      case tlv::MustBeFresh: {
        if (lastElement >= 3) {
          BOOST_THROW_EXCEPTION(Error("MustBeFresh element is out of order"));
        }
        if (element->value_size() != 0) {
          BOOST_THROW_EXCEPTION(Error("MustBeFresh element has non-zero TLV-LENGTH"));
        }
        m_selectors.setMustBeFresh(true);
        lastElement = 3;
        break;
      }
      case tlv::ForwardingHint: {
        if (lastElement >= 4) {
          BOOST_THROW_EXCEPTION(Error("ForwardingHint element is out of order"));
        }
        m_forwardingHint.wireDecode(*element);
        lastElement = 4;
        break;
      }
      case tlv::Nonce: {
        if (lastElement >= 5) {
          BOOST_THROW_EXCEPTION(Error("Nonce element is out of order"));
        }
        uint32_t nonce = 0;
        if (element->value_size() != sizeof(nonce)) {
          BOOST_THROW_EXCEPTION(Error("Nonce element is malformed"));
        }
        std::memcpy(&nonce, element->value(), sizeof(nonce));
        m_nonce = nonce;
        lastElement = 5;
        break;
      }
      case tlv::InterestLifetime: {
        if (lastElement >= 6) {
          BOOST_THROW_EXCEPTION(Error("InterestLifetime element is out of order"));
        }
        m_interestLifetime = time::milliseconds(readNonNegativeInteger(*element));
        lastElement = 6;
        break;
      }
      case tlv::HopLimit: {
        if (lastElement >= 7) {
          break; // HopLimit is non-critical, ignore out-of-order appearance
        }
        if (element->value_size() != 1) {
          BOOST_THROW_EXCEPTION(Error("HopLimit element is malformed"));
        }
        // TLV-VALUE is ignored
        lastElement = 7;
        break;
      }
      case tlv::Parameters: {
        if (lastElement >= 8) {
          BOOST_THROW_EXCEPTION(Error("Parameters element is out of order"));
        }
        m_parameters = *element;
        lastElement = 8;
        break;
      }
      default: {
        if (tlv::isCriticalType(element->type())) {
          BOOST_THROW_EXCEPTION(Error("unrecognized element of critical type " +
                                      to_string(element->type())));
        }
        break;
      }
    }
  }
}

std::string Interest::toUri() const{
  std::ostringstream os;
  os << *this;
  return os.str();
}

// ---- matching ----

bool Interest::matchesName(const Name& name) const{
  // 首先比较两个name的大小
  if (name.size() < m_name.size())
    return false;

  // 判断m_name是否是name的前缀
  if (!m_name.isPrefixOf(name))
    return false;

  if (getMinSuffixComponents() >= 0 &&
      // name must include implicit digest
      !(name.size() - m_name.size() >= static_cast<size_t>(getMinSuffixComponents())))
    return false;

  if (getMaxSuffixComponents() >= 0 &&
      // name must include implicit digest
      !(name.size() - m_name.size() <= static_cast<size_t>(getMaxSuffixComponents())))
    return false;

  if (!getExclude().empty() &&
      name.size() > m_name.size() &&
      getExclude().isExcluded(name[m_name.size()]))
    return false;

  return true;
}

bool Interest::matchesData(const Data& data) const{

  // 兴趣包名称m_name的大小
  size_t interestNameLength = m_name.size();
  // 获取数据包的名称dataName
  const Name& dataName = data.getName();
  // 全名的长度包括最后的隐式摘要
  size_t fullNameLength = dataName.size() + 1;

  // check MinSuffixComponents
  bool hasMinSuffixComponents = getMinSuffixComponents() >= 0;
  size_t minSuffixComponents = hasMinSuffixComponents ?
                               static_cast<size_t>(getMinSuffixComponents()) : 0;
  if (!(interestNameLength + minSuffixComponents <= fullNameLength))
    return false;

  // check MaxSuffixComponents
  bool hasMaxSuffixComponents = getMaxSuffixComponents() >= 0;
  if (hasMaxSuffixComponents &&
      !(interestNameLength + getMaxSuffixComponents() >= fullNameLength))
    return false;

  // check prefix
  if (interestNameLength == fullNameLength) {
    // m_name最后的组件是隐式摘要
    if (m_name.get(-1).isImplicitSha256Digest()) {
      if (m_name != data.getFullName())
        return false;
    }
    else {
      // Interest Name与Data full Name长度相同，但最后一个组件没有摘要，因此不可能匹配
      return false;
    }
  }
  else {
    // m_name是fullname的严格前缀
    if (!m_name.isPrefixOf(dataName))
      return false;
  }

  // check Exclude
  // Exclude won't be violated if Interest Name is same as Data full Name
  if (!getExclude().empty() && fullNameLength > interestNameLength) {
    if (interestNameLength == fullNameLength - 1) {
      // component to exclude is the digest
      if (getExclude().isExcluded(data.getFullName().get(interestNameLength)))
        return false;
      // There's opportunity to inspect the Exclude filter and determine whether
      // the digest would make a difference.
      // eg. "<GenericNameComponent>AA</GenericNameComponent><Any/>" doesn't exclude
      //     any digest - fullName not needed;
      //     "<Any/><GenericNameComponent>AA</GenericNameComponent>" and
      //     "<Any/><ImplicitSha256DigestComponent>ffffffffffffffffffffffffffffffff
      //      </ImplicitSha256DigestComponent>"
      //     excludes all digests - fullName not needed;
      //     "<Any/><ImplicitSha256DigestComponent>80000000000000000000000000000000
      //      </ImplicitSha256DigestComponent>"
      //     excludes some digests - fullName required
      // But Interests that contain the exact Data Name before digest and also
      // contain Exclude filter is too rare to optimize for, so we request
      // fullName no matter what's in the Exclude filter.
    }
    else {
      // component to exclude is not the digest
      if (getExclude().isExcluded(dataName.get(interestNameLength)))
        return false;
    }
  }

  // check PublisherPublicKeyLocator
  const KeyLocator& publisherPublicKeyLocator = this->getPublisherPublicKeyLocator();
  if (!publisherPublicKeyLocator.empty()) {
    const Signature& signature = data.getSignature();
    const Block& signatureInfo = signature.getInfo();
    Block::element_const_iterator it = signatureInfo.find(tlv::KeyLocator);
    if (it == signatureInfo.elements_end()) {
      return false;
    }
    if (publisherPublicKeyLocator.wireEncode() != *it) {
      return false;
    }
  }

  return true;
}

bool Interest::matchesInterest(const Interest& other) const{

  // 比较name和Selectors
  return (this->getName() == other.getName() &&
          this->getSelectors() == other.getSelectors());
}

// ---- field accessors ----
/******************************nonce*************************************/
uint32_t Interest::getNonce() const{
  // m_nonce不存在，被添加并赋值为随机生成的值
  if (!m_nonce) {
    m_nonce = random::generateWord32();
  }
  return *m_nonce;
}

Interest& Interest::setNonce(uint32_t nonce){
  m_nonce = nonce;
  m_wire.reset();
  return *this;
}

void Interest::refreshNonce(){
  // 没有nonce，什么也不做
  if (!hasNonce())
    return;

  // nonce的值不同于旧值
  uint32_t oldNonce = getNonce();
  uint32_t newNonce = oldNonce;
  while (newNonce == oldNonce)
    newNonce = random::generateWord32();

  setNonce(newNonce);
}
/******************************nonce*************************************/


/******************************InterestLifetime*************************************/
Interest& Interest::setInterestLifetime(time::milliseconds lifetime){
  // lifetime小于零，抛出异常std::invalid_argument
  if (lifetime < time::milliseconds::zero()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("InterestLifetime must be >= 0"));
  }
  // 设置lifetime
  m_interestLifetime = lifetime;
  m_wire.reset();
  return *this;
}
/******************************InterestLifetime*************************************/

/******************************ForwardingHint*************************************/
Interest& Interest::setForwardingHint(const DelegationList& value){
  m_forwardingHint = value;
  m_wire.reset();
  return *this;
}
/******************************ForwardingHint*************************************/


/******************************Parameters*************************************/
// Set parameters from a Block
Interest& Interest::setParameters(const Block& parameters){
  // 如果块的TLV-TYPE是Parameters，它将被直接用作Interest的Parameters元素。
  if (parameters.type() == tlv::Parameters) {
    m_parameters = parameters;
  }
  else {
    // 如果块的TLV-TYPE不是Parameters，它将被嵌套到Parameters元素中。
    m_parameters = Block(tlv::Parameters, parameters);
  }
  m_wire.reset();
  return *this;
}

// Copy parameters from raw buffer
Interest& Interest::setParameters(const uint8_t* buffer, size_t bufferSize){
  m_parameters = makeBinaryBlock(tlv::Parameters, buffer, bufferSize);
  m_wire.reset();
  return *this;
}

//  Set parameters from a wire buffer
Interest& Interest::setParameters(ConstBufferPtr buffer){
  m_parameters = Block(tlv::Parameters, std::move(buffer));
  m_wire.reset();
  return *this;
}

// 移除Parameters
Interest& Interest::unsetParameters(){
  m_parameters = Block();
  m_wire.reset();
  return *this;
}
/******************************Parameters*************************************/

// ---- operators ----

bool operator==(const Interest& lhs, const Interest& rhs){
  bool wasCanBePrefixSetOnLhs = lhs.m_isCanBePrefixSet;
  bool wasCanBePrefixSetOnRhs = rhs.m_isCanBePrefixSet;
  lhs.m_isCanBePrefixSet = true;
  rhs.m_isCanBePrefixSet = true;
  BOOST_SCOPE_EXIT_ALL(&) {
    lhs.m_isCanBePrefixSet = wasCanBePrefixSetOnLhs;
    rhs.m_isCanBePrefixSet = wasCanBePrefixSetOnRhs;
  };

  return lhs.wireEncode() == rhs.wireEncode();
}

std::ostream& operator<<(std::ostream& os, const Interest& interest){
  // 输出name
  os << interest.getName();

  // 定义分隔符
  char delim = '?';

  if (interest.getMinSuffixComponents() >= 0) {
    os << delim << "ndn.MinSuffixComponents=" << interest.getMinSuffixComponents();
    delim = '&';
  }
  if (interest.getMaxSuffixComponents() >= 0) {
    os << delim << "ndn.MaxSuffixComponents=" << interest.getMaxSuffixComponents();
    delim = '&';
  }
  if (interest.getChildSelector() != DEFAULT_CHILD_SELECTOR) {
    os << delim << "ndn.ChildSelector=" << interest.getChildSelector();
    delim = '&';
  }
  if (interest.getMustBeFresh()) {
    os << delim << "ndn.MustBeFresh=" << interest.getMustBeFresh();
    delim = '&';
  }
  if (interest.getInterestLifetime() != DEFAULT_INTEREST_LIFETIME) {
    os << delim << "ndn.InterestLifetime=" << interest.getInterestLifetime().count();
    delim = '&';
  }

  if (interest.hasNonce()) {
    os << delim << "ndn.Nonce=" << interest.getNonce();
    delim = '&';
  }
  if (!interest.getExclude().empty()) {
    os << delim << "ndn.Exclude=" << interest.getExclude();
    delim = '&';
  }

  return os;
}

} // namespace ndn
```
