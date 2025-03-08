ndn-cxx/name-component.hpp
```cpp
namespace ndn {
namespace name {

/** Represents a name component.
 *
 *  The Component class provides a read-only view of a Block interpreted as a name component.
 *  Although it inherits mutation methods from Block base class, they must not be used, because
 *  the enclosing Name would not be updated correctly.
 */
// 表示一个名称组件name component。
// Component类提供了Block的只读视图，它被解释为一个名称组件。
// 虽然它从Block基类继承了突变方法，但它们不能被使用，因为封闭的Name不会被正确更新。
class Component : public Block{
};

}
} // namespace ndn
```

ndn-cxx/name.hpp
```cpp
namespace ndn {

class Name;


using PartialName = Name;

// an absolute name
class Name
{
public: // nested types
/*******************************************************************/
  using Error = name::Component::Error;

  using Component = name::Component;
  using component_container = std::vector<Component>;

  // Name appears as a container of name components
  using value_type             = Component;
  using allocator_type         = void;
  using reference              = Component&;
  using const_reference        = const Component&;
  using pointer                = Component*;
  using const_pointer          = const Component*;
  using iterator               = const Component*; // disallow modifying via iterator
  using const_iterator         = const Component*;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using difference_type        = component_container::difference_type;
  using size_type              = component_container::size_type;
/************************************************************************/

public: // constructors, encoding, decoding

  // 默认构造函数;使用此构造函数创建实例会使得empty() == true;
  Name();

  /** 解码Name从wire encoding
   *  @throw
      tlv::Error ： wire encoding is invalid
   *  这是一个更高效的等价：
   *  @code
   *    Name name;
   *    name.wireDecode(wire);
   *  @endcode
   */
  // 显示构造函数
  explicit Name(const Block& wire);

  /** 解析name从NDN URI
   *  uri ： a null-terminated URI string
   *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#ndn-uri-scheme
   */
  Name(const char* uri);

  /** 新建name从NDN URI
   *  uri ： a URI string
   *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#ndn-uri-scheme
   */
  Name(std::string uri);

  /** 获取表示name的URI
   *  @return URI representation; "ndn:" scheme identifier is not included
   *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#ndn-uri-scheme
   *  @note To print URI representation into a stream, it is more efficient to use ``os << name``.
   */
  std::string toUri() const;

  // 检查Name实例是否含有wire encoding
  bool hasWire() const{
    return m_wire.hasWire();
  }

  // Fast encoding or block size estimation
  template<encoding::Tag TAG>
  size_t wireEncode(EncodingImpl<TAG>& encoder) const;

  // @brief Perform wire encoding, or return existing wire encoding
  // @post hasWire() == true
  const Block& wireEncode() const;

  // Decode name from wire encoding
  // @throw tlv::Error ： wire encoding is invalid
  // @post hasWire() == true
  void wireDecode(const Block& wire);

  // 生成name的深层副本，重新分配底层内存缓冲区
  Name deepCopy() const;

public: // access

  // Check if name is empty
  bool empty() const{
    return m_wire.elements().empty();
  }

  // Get number of components
  size_t size() const{
    return m_wire.elements_size();
  }

  // Get the component at the given index
  // @param i ： zero-based index; if negative, it starts at the end of this name
  // @warning Indexing out of bounds triggers undefined behavior.
  const Component& get(ssize_t i) const{
    if (i < 0) {
      i += size();
    }
    return reinterpret_cast<const Component&>(m_wire.elements()[i]);
  }

  // Equivalent to get(i)
  const Component& operator[](ssize_t i) const{
    return get(i);
  }

  /** @brief Get the component at the given index
   *  @param i zero-based index; if negative, size()+i is used instead
   *  @throws Name::Error index is out of bounds
   */
  const Component& at(ssize_t i) const;


  // 提取一些components作为sub-name(PartialName)
  // iStartComponent： 如果为负值，用size()+iStartComponent进行校正
  // nComponents： components的个数；默认是npos表示整个name
  // 如果iStartComponent为正数并且索引超出边界，则返回一个空的PartialName。
  // 如果iStartComponent为负且索引越界，则返回从Name开头开始的components。
  // 如果nComponents越界，则返回components直到该Name的末尾。
  PartialName getSubName(ssize_t iStartComponent, size_t nComponents = npos) const;



  // 提取name的prefix，并返回一个新的partial name
  // nComponents： components的个数，如果为负值用size()+nComponents进行校正
  PartialName getPrefix(ssize_t nComponents) const{
    if (nComponents < 0)
      return getSubName(0, size() + nComponents);
    else
      return getSubName(0, nComponents);
  }

public: // iterators

  // Begin iterator
  const_iterator begin() const{
    return reinterpret_cast<const_iterator>(m_wire.elements().data());
  }

  // End iterator
  const_iterator end() const{
    return reinterpret_cast<const_iterator>(m_wire.elements().data() + m_wire.elements().size());
  }

  // Reverse begin iterator
  const_reverse_iterator rbegin() const{
    return const_reverse_iterator(end());
  }

  // Reverse end iterator
  const_reverse_iterator rend() const{
    return const_reverse_iterator(begin());
  }

public: // modifiers

  // Append a component.
  Name& append(const Component& component){
    m_wire.push_back(component);
    return *this;
  }

  // Append a NameComponent of [TLV-TYPE：type], copying [count] bytes at value as [TLV-VALUE].
  Name& append(uint32_t type, const uint8_t* value, size_t count){
    return append(Component(type, value, count));
  }

  // Append a GenericNameComponent, copying [count] bytes at value as [TLV-VALUE].
  Name& append(const uint8_t* value, size_t count){
    return append(Component(value, count));
  }

  /** Append a NameComponent of [TLV-TYPE: type], copying TLV-VALUE from a range.
   *  @tparam
      Iterator : 当它是一个RandomAccessIterator时，可以获得更高效的实现。
   *  type   :   the TLV-TYPE.
   *  first  :   beginning of the range.
   *  last  :    past-end of the range.
   */
  template<class Iterator>
  Name& append(uint32_t type, Iterator first, Iterator last){
    return append(Component(type, first, last));
  }

  /** Append a GenericNameComponent, copying TLV-VALUE from a range.
   *  @tparam Iterator ： an InputIterator dereferencing to a one-octet value type.
                          当它是一个RandomAccessIterator时，可以获得更高效的实现。
   *  @param
      first    ： beginning of the range.
   *  last  ：    past-end of the range.
   */
  template<class Iterator>
  Name& append(Iterator first, Iterator last){

    return append(Component(first, last));
  }

  /** Append a GenericNameComponent, copying TLV-VALUE from a null-terminated string.
   *  str： a null-terminated string. 字符串中的字节被原样复制，而不被解释为URI组件。
   */
  Name& append(const char* str){

    return append(Component(str));
  }

  /** Append a GenericNameComponent from a TLV element.
   *  value ： a TLV element. 如果它的类型是tlv::GenericNameComponent，则按原样使用它。
                            否则，它将被封装到GenericNameComponent中。
   */
  Name& append(const Block& value){

    if (value.type() == tlv::GenericNameComponent) {
      m_wire.push_back(value);
    }
    else {
      m_wire.push_back(Block(tlv::GenericNameComponent, value));
    }
    return *this;
  }

  // Append a component with a nonNegativeInteger
  // @sa https://named-data.net/doc/NDN-packet-spec/current/tlv.html#non-negative-integer-encoding
  Name& appendNumber(uint64_t number){

    return append(Component::fromNumber(number));
  }

  /** Append a component with a marked number
   *  marker ： 1-octet marker；[octet:八位字节，相当于一个Bytes]
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendNumberWithMarker(uint8_t marker, uint64_t number){

    return append(Component::fromNumberWithMarker(marker, number));
  }

  /** Append a version component
   *  version ： 要附加的版本号;如果为空，则使用当前UNIX时间(以毫秒为单位)
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendVersion(optional<uint64_t> version = nullopt);

  /** Append a segment number (sequential) component
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendSegment(uint64_t segmentNo){

    return append(Component::fromSegment(segmentNo));
  }

  /** Append a segment byte offset component
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendSegmentOffset(uint64_t offset){

    return append(Component::fromSegmentOffset(offset));
  }

  /** Append a timestamp component
   *  timestamp ： the timestamp to append; if nullopt, the current system time is use
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendTimestamp(optional<time::system_clock::TimePoint> timestamp = nullopt);

  /** Append a sequence number component
   *  @sa NDN Naming Conventions https://named-data.net/doc/tech-memos/naming-conventions.pdf
   */
  Name& appendSequenceNumber(uint64_t seqNo){

    return append(Component::fromSequenceNumber(seqNo));
  }

  // Append an ImplicitSha256Digest component
  Name& appendImplicitSha256Digest(ConstBufferPtr digest){

    return append(Component::fromImplicitSha256Digest(std::move(digest)));
  }

  // Append an ImplicitSha256Digest component
  Name& appendImplicitSha256Digest(const uint8_t* digest, size_t digestSize){

    return append(Component::fromImplicitSha256Digest(digest, digestSize));
  }

  // Append a PartialName
  Name& append(const PartialName& name);

  // Append a component
  // 这使得push_back成为append的别名，为Name提供了与STL vector类似的API。
  template<class T>
  void push_back(const T& component){
    append(component);
  }


  // 删除所有components会导致empty() == true
  void clear(){
    m_wire = Block(tlv::Name);
  }

public: // algorithms
  /** Get the successor of a name
   *
   *  The successor of a name is defined as follows:
   *
   *      N represents the set of NDN Names, and X,Y ∈ N.
   *      Operator < is defined by canonical order on N.
   *      Y is the successor of X, if (a) X < Y, and (b) ∄ Z ∈ N s.t. X < Z < Y.
   *
   *  In plain words, successor of a name is the same name, but with its last component
   *  advanced to a next possible value.
   *
   *  Examples:
   *
   *  - successor of `/` is
   *    `/sha256digest=0000000000000000000000000000000000000000000000000000000000000000`.
   *  - successor of `/sha256digest=0000000000000000000000000000000000000000000000000000000000000000`
   *    is `/sha256digest=0000000000000000000000000000000000000000000000000000000000000001`.
   *  - successor of `/sha256digest=ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff`
   *    is `/2=...`.
   *  - successor of `/P/A` is `/P/B`.
   *  - successor of `/Q/%FF` is `/Q/%00%00`.
   *
   *  @return a new Name containing the successor
   */
  Name getSuccessor() const;

  /** 检查this name是否为other name的前缀
   *  this name 是 other name前缀的条件： this name的N个components 等于 other的N个components
   *  @retval
      true ： this name is a prefix of other
   *  false ： this name is not a prefix of other
   */
  bool isPrefixOf(const Name& other) const;

  // 相等的条件：
  // the same number of components
  // components at each index are equal.
  bool equals(const Name& other) const;

  /** 将this与使用NDN规范排序的other进行比较。
   *
   *  1.If the first components of each name are not equal,
      this returns a negative value if the first comes before the second using the NDN canonical ordering for name components,
      or a positive value if it comes after.  
   *  2.If they are equal,
      this compares the second components of each name, etc.
      If both names are the same up to the size of the shorter name,
      this returns a negative value if the first name is shorter than the second
      or a positive value if it is longer.  
      3.For example, if you std::sort gives:
   *  /a/b/d /a/b/cc /c /c/a /bb .
   *  这很直观，因为所有以 `/a` 为前缀的名称都彼此相邻。
   *  但另一方面，根据NDN的标准排序规则，这也可能不太直观，因为 `/c` 比 `/bb` 排在前面，因为它更短。

   *  @retval
      negative ： this comes before other in canonical ordering
   *  zero ： this equals other
   *  positive ： this comes after other in canonical ordering
   *
   *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#canonical-order
   */
  int compare(const Name& other) const{
    return this->compare(0, npos, other);
  }

  // this[pos1,pos1+count1)与other[pos2,pos2+count2)比较
  // 等效于this->getSubName(pos1, count1).compare(other.getSubName(pos2, count2));
  int compare(size_t pos1, size_t count1, const Name& other, size_t pos2 = 0, size_t count2 = npos) const;

public:
  // 该字段有以下两种用途：
  // indicates "until the end" in getSubName
  // compare
  static const size_t npos;

private:
  mutable Block m_wire;
};

/********************************************************/
NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(Name);

inline bool operator==(const Name& lhs, const Name& rhs){
  return lhs.equals(rhs);
}

inline bool operator!=(const Name& lhs, const Name& rhs){
  return !lhs.equals(rhs);
}

inline bool operator<=(const Name& lhs, const Name& rhs){
  return lhs.compare(rhs) <= 0;
}

inline bool operator<(const Name& lhs, const Name& rhs){
  return lhs.compare(rhs) < 0;
}

inline bool operator>=(const Name& lhs, const Name& rhs){
  return lhs.compare(rhs) >= 0;
}

inline bool operator>(const Name& lhs, const Name& rhs){
  return lhs.compare(rhs) > 0;
}

/** @brief Print URI representation of a name
 *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#ndn-uri-scheme
 */
std::ostream& operator<<(std::ostream& os, const Name& name);

/** @brief Parse URI from stream as Name
 *  @sa https://named-data.net/doc/NDN-packet-spec/current/name.html#ndn-uri-scheme
 */
std::istream& operator>>(std::istream& is, Name& name);
/*****************************************************************************/
} // namespace ndn

/**************************************************************************/
namespace std {

template<>
struct hash<ndn::Name>{
  size_t operator()(const ndn::Name& name) const;
};

} // namespace std
/**************************************************************************/
```
