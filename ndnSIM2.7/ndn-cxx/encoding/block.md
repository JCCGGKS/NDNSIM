ndn-cxx/encoding/block.hpp
```cpp
namespace ndn {

/** @brief Represents a TLV element of NDN packet format
 *  @sa https://named-data.net/doc/ndn-tlv/tlv.html
 */
class Block
{
public:
  /************************vector<Block>********************************/
  using element_container      = std::vector<Block>;
  using element_iterator       = element_container::iterator;
  using element_const_iterator = element_container::const_iterator;
  /********************************************************************/



public: // constructor, creation, assignment
  //构造函数
  Block();

  //拷贝构造函数
  Block(const Block&);

  //拷贝赋值函数
  Block& operator=(const Block&);

  //移动构造函数                   
  Block(Block&&) noexcept;

  //移动赋值函数
  Block& operator=(Block&&) noexcept;

  // 从EncodingBuffer中解析Block
  explicit Block(const EncodingBuffer& buffer);

  /** 从wire Buffer中解析Block
   *  buffer ：a Buffer containing one TLV element
   *  这个构造函数共享缓冲区的所有权。
   */
  explicit Block(const ConstBufferPtr& buffer);

  /** 在wire buffer的边界内解析Block
   *  @param
      buffer ：a Buffer containing an TLV element at [begin,end)
   *  begin： begin position of the TLV element within buffer
   *  end： end position of the TLV element within buffer
   *  verifyLength if true, check TLV-LENGTH equals size of TLV-VALUE
   *  @throw
      std::invalid_argument：buffer is empty, or [begin,end) range are not within buffer
   *  tlv::Error：Type-Length parsing fails, or TLV-LENGTH does not match size of TLV-VALUE
   *  @note
      自动检测TLV-TYPE and TLV-VALUE的位置.
   */
  Block(ConstBufferPtr buffer, Buffer::const_iterator begin, Buffer::const_iterator end,bool verifyLength = true);

  /** 在现有块的边界内解析块，重用底层wire buffer
   *  @param
      block： a Block whose buffer contains an TLV element at [begin, end)
   *  begin： begin position of the TLV element within  block
   *  end： end position of the TLV element within  block
   *  verifyLength if true, check TLV-LENGTH equals size of TLV-VALUE
   *  @throw
      std::invalid_argument：[begin, end) range are not within  block
   *  tlv::Error： Type-Length parsing fails, or TLV-LENGTH does not match size of TLV-VALUE
   */
  Block(const Block& block, Buffer::const_iterator begin, Buffer::const_iterator end,bool verifyLength = true);


  /** 在没有解析的情况下从wire buffer中创建Block
   *  @param
      buffer： a Buffer containing an TLV element at [begin,end)
   *  type： TLV-TYPE
   *  begin： begin position of the TLV element within buffer
   *  end： end position of the TLV element within buffer
   *  valueBegin： begin position of TLV-VALUE within buffer
   *  valueEnd： end position of TLV-VALUE within buffer
   */
  Block(ConstBufferPtr buffer, uint32_t type,
        Buffer::const_iterator begin, Buffer::const_iterator end,
        Buffer::const_iterator valueBegin, Buffer::const_iterator valueEnd);

  /** 从raw buffer中解析块
   *  @param
      buf： pointer to the first octet of an TLV element
   *  bufSize： size of the raw buffer; may be more than size of the TLV element
   *  @throw
      tlv::Error Type-Length parsing fails, or size of TLV-VALUE exceeds bufSize
   *  @note
      将TLV元素复制到内部wire buffer
   */
  Block(const uint8_t* buf, size_t bufSize);


  // 用特定的TLV-TYPE值创建空白Block
  // type： TLV-TYPE
  explicit Block(uint32_t type);

  /** 用特定的TLV-TYPE and TLV-VALUE创建空白Block
   *  @param
      type： TLV-TYPE
   *  value： a Buffer containing the TLV-VALUE
   */
  Block(uint32_t type, ConstBufferPtr value);


  /** 用特定的TLV-TYPE and TLV-VALUE创建空白Block
   *  @param
      type： TLV-TYPE
   *  value： a Block to be nested as TLV-VALUE
   */
  Block(uint32_t type, const Block& value);


  /** 从input stream中解析Block
   *  @throw
      tlv::Error： TLV-LENGTH is zero or exceeds upper bound
   *  @warning
      If decoding fails, bytes are still consumed from the input stream.
   */
  static Block fromStream(std::istream& is);


  /** 尝试从wire buffer中解析Block
   *  @param
      buffer： a Buffer containing an TLV element at offset offset
   *  offset： begin position of the TLV element within buffer
   *  @note
      This function does not throw exceptions upon decoding failure.
   *  @return
      true and the Block if parsing succeeds; otherwise false
   */
  static std::tuple<bool, Block> fromBuffer(ConstBufferPtr buffer, size_t offset);


  /** 尝试从raw buffer中解析Block
   *  @param
      buf： pointer to the first octet of an TLV element
   *  bufSize： size of the raw buffer; may be more than size of the TLV element
   *  @note
      This function does not throw exceptions upon decoding failure.
   *  This overload copies the TLV element into an internal wire buffer.
   *  @return
      true and the Block if parsing succeeds; otherwise false
   */
  static std::tuple<bool, Block> fromBuffer(const uint8_t* buf, size_t bufSize);

public: // wire format

  // 块只有在默认构造时才为“空”。TLV-VALUE长度为零的块不被认为是空的。
  bool empty() const{
    return m_type == std::numeric_limits<uint32_t>::max();
  }

  // 判断Block是否含有fully encoded wire
  // 如果有的话，底层buffer存在并且包含完整的Type-Length-Value字段，而不仅仅是TLV-VALUE字段。
  bool hasWire() const;


  // 重新设置wire buffer的element
  // @post empty() == true
  void reset();


  // 重新设置wire buffer但是保持TLV-TYPE和sub elements(如果有的话)
  // @post hasWire() == false
  // @post hasValue() == false
  void resetWire();

  // 获取encoded wire的begin iterator
  // 前提：hasWire() == true
  Buffer::const_iterator begin() const;


  // 获取encoded wire的end iterator
  // 前提：hasWire() == true
  Buffer::const_iterator end() const;


  // 获取指向encoded wire的指针
  // 前提：hasWire() == true
  const uint8_t* wire() const;


  // 获取encoded wire的大小，包括Type-Length-Value
  // 前提：empty() == false
  size_t size() const;


  // Get underlying buffer
  shared_ptr<const Buffer> getBuffer() const{
    return m_buffer;
  }

public: // type and value

  // Get TLV-TYPE
  uint32_t type() const{
    return m_type;
  }


  // 此属性反映底层Buffer是否包含TLV-VALUE。
  // 如果为false, TLV-VALUE的长度为零。如果为真，TLV-VALUE可能为零长度。
  bool hasValue() const{
    return m_buffer != nullptr;
  }

  // Get begin iterator of TLV-VALUE
  // 准备工作：hasValue() == true
  Buffer::const_iterator value_begin() const{
    return m_valueBegin;
  }

  // Get end iterator of TLV-VALUE
  // 准备工作：hasValue() == true
  Buffer::const_iterator value_end() const{
    return m_valueEnd;
  }

  // 获取TLV-VALUE
  const uint8_t* value() const;

  // 获取TLV-VALUE的大小，即TLV-LENGTH
  size_t value_size() const;

  Block blockFromValue() const;

public: // sub elements

  /** 将TLV-VALUE解析成成sub elements
   *  @post elements() reflects sub elements found in TLV-VALUE
   *  @throw tlv::Error ： TLV-VALUE is not a sequence of TLV elements
   *  @note
      此方法不执行递归解析。
      如果elements()已经被填充，则此方法不起作用。
      这个方法不是真正的const，但它不修改任何数据。
   */
  void parse() const;

  /** 将sub elements编码成TLV-VALUE
   *  @post TLV-VALUE contains sub elements from elements()
   */
  void encode();

  /** @brief Get the first sub element of specified TLV-TYPE
   *  准备工作：parse() has been executed
   *  @throw Error sub element of @p type does not exist
   */
  const Block& get(uint32_t type) const;

  /** Find the first sub element of specified TLV-TYPE
   *  准备工作：parse() has been executed
   *  @return iterator in elements() to the found sub element, otherwise elements_end()
   */
  element_const_iterator find(uint32_t type) const;

  /** Remove all sub elements of specified TLV-TYPE
   *  准备工作：parse() has been executed
   *  @post find(type) == elements_end()
   */
  void remove(uint32_t type);

  // Erase a sub element
  element_iterator erase(element_const_iterator position);

  // Erase a range of sub elements
  element_iterator erase(element_const_iterator first, element_const_iterator last);

  // Append a sub element
  void push_back(const Block& element);

  /** Insert a sub element
   *  @param
      pos ： position of new sub element
   *  element ： new sub element
   *  @return iterator in elements() to the new sub element
   */
  element_iterator insert(element_const_iterator pos, const Block& element);

  // 获取包含sub elements的容器
  // 准备工作：parse() has been executed
  const element_container& elements() const{
    return m_elements;
  }

  // 等效于elements().begin()
  element_const_iterator elements_begin() const{
    return m_elements.begin();
  }

  // 等效于elements().end()
  element_const_iterator elements_end() const{
    return m_elements.end();
  }


  // 等效于elements().size()
  size_t elements_size() const{
    return m_elements.size();
  }

public: // misc

  // 隐式转换为const_buffer类型
  operator boost::asio::const_buffer() const;

private:
  // 估计Block size，就像sub elements被编码成TLV-VALUE一样
  size_t encode(EncodingEstimator& estimator) const;


  // 估计TLV-LENGTH，就像sub elements被编码成TLV-VALUE
  size_t encodeValue(EncodingEstimator& estimator) const;

  /** 将子元素编码为TLV-VALUE
   *  @post
      TLV-VALUE 热色图contains sub elements from elements()
   *  internal buffer and iterators point to Encoder's buffer
   */
  size_t encode(EncodingBuffer& encoder);

protected:

  /*
  * 底层buffer存储TLV-VALUE、TLY-TYPE(可能)、TLV-LENGTH字段
  * 如果m_buffer是nullptr，这是一个带有TLV-TYPE的空Block
  * [m_valueBegin,m_valueEnd)指向m_buffer中的TLV_VALUE
  * 如果m_begin!=m_end，[m_begin,m_end)指向Type-Length-Value
  * 另外的情况，m_buffer不包含TLV-TYPE and TLV-LENGTH字段
  */
  shared_ptr<const Buffer> m_buffer;

  Buffer::const_iterator m_begin;
  Buffer::const_iterator m_end;

  Buffer::const_iterator m_valueBegin;
  Buffer::const_iterator m_valueEnd;

  uint32_t m_type = std::numeric_limits<uint32_t>::max(); ///< TLV-TYPE

  // 包括Type-Length-Value在内的总大小
  // 该字段仅在empty()为false时有效。
  size_t m_size = 0;

  // 该字段仅在parse()被执行时有效
  // using element_container      = std::vector<Block>;
  mutable element_container m_elements;

  /** Print  block to os.
   * 默认构造的块被打印为:' [invalid] '。
   * 零长度块被打印为:' TT[empty] '，其中TT是十进制的TLV-TYPE。
   * 未调用block::parse的非零长度块打印为:' TT[LL]=VVVV '，其中LL是十进制的TLV-LENGTH, VVVV是十六进制的TLV-VALUE。
   * Block::parse被调用的块被打印为:' TT[LL]={SUB,SUB} '，其中SUB是使用此格式打印的子元素。
   */
  friend std::ostream& operator<<(std::ostream& os, const Block& block);
};

/******************************************************************************/
inline Block::Block(Block&&) noexcept = default;

inline Block& Block::operator=(Block&&) noexcept = default;

// 比较两个Block是否有相同的TLV-TYPE、TLV-LENGTH、TLV-
bool operator==(const Block& lhs, const Block& rhs);

inline bool operator!=(const Block& lhs, const Block& rhs){
  return !(lhs == rhs);
}

/** 从十六进制输入构造一个Block
 *  \param
    input ： a string containing hexadecimal bytes and comments.
 *               0-9 and upper-case A-F are input; all other characters are comments.
 *  len ： length of input.
 *  \throw
    std::invalid_argument ： input is empty or has odd number of hexadecimal digits.
 *  tlv::Error ： input cannot be parsed into valid  Block.
 */

 /*  Example
 *  \code
 *  Block nameBlock = "0706 080141 080142"_block;
 *  Block nackBlock = "FD032005 reason(no-route)=FD03210196"_block;
 *  \endcode
 */
Block operator "" _block(const char* input, std::size_t len);

} // namespace ndn
```

每一个NDN Packet按照“TLV”的格式进行编码，详细介绍请参考[Type-Length-Value encoding](https://docs.named-data.net/NDN-packet-spec/current/tlv.html)

ndn-cxx/block.cpp
```cpp
namespace ndn{
/***************************wire format***********************************/
  // 判断实例化对象是否含有wire
  bool Block::hasWire() const{
    return m_buffer != nullptr && m_begin != m_end;
  }

  void Block::reset(){
    this->resetWire();

    m_type = std::numeric_limits<uint32_t>::max();
    // 调用的是std::vector<Block>::clear()
    m_elements.clear();
  }

  void Block::resetWire(){
    // 调用的是std::shared_ptr::reset()删除原本托管的对象
    m_buffer.reset(); // discard underlying buffer by resetting shared_ptr
    m_begin = m_end = m_valueBegin = m_valueEnd = {};
  }

  Buffer::const_iterator Block::begin() const{
    if (!hasWire())
      BOOST_THROW_EXCEPTION(Error("Underlying wire buffer is empty"));

    return m_begin;
  }

  Buffer::const_iterator Block::end() const{
    if (!hasWire())
      BOOST_THROW_EXCEPTION(Error("Underlying wire buffer is empty"));

    return m_end;
  }

  const uint8_t* Block::wire() const{
    if (!hasWire())
      BOOST_THROW_EXCEPTION(Error("Underlying wire buffer is empty"));

    //Buffer是一种vector<uint8_t>
    //m_begin的本质是vector<uint8_t>::const_iterator
    return &*m_begin;
  }

  size_t Block::size() const{
    if (empty()) {
      BOOST_THROW_EXCEPTION(Error("Block size cannot be determined (undefined block size)"));
    }

    return m_size;
  }
}
/***************************************wire format**************************/

/*********************************value*************************************/
const uint8_t* Block::value() const{
  // hasValue : m_buffer!=nullptr
  return hasValue() ? &*m_valueBegin : nullptr;
}


size_t Block::value_size() const{
  return hasValue() ? static_cast<size_t>(m_valueEnd - m_valueBegin) : 0;
}


Block Block::blockFromValue() const{
  if (!hasValue())
    BOOST_THROW_EXCEPTION(Error("Block has no TLV-VALUE"));

  return Block(*this, m_valueBegin, m_valueEnd, true);
}
/*********************************value*************************************/

/************************************sub elements***************************/
void Block::parse() const{
  if (!m_elements.empty() || value_size() == 0)
    return;

  Buffer::const_iterator begin = value_begin();
  Buffer::const_iterator end = value_end();

  while (begin != end) {
    Buffer::const_iterator pos = begin;

    uint32_t type = tlv::readType(pos, end);
    uint64_t length = tlv::readVarNumber(pos, end);
    if (length > static_cast<uint64_t>(end - pos)) {
      m_elements.clear();
      BOOST_THROW_EXCEPTION(Error("TLV-LENGTH of sub-element of type " + to_string(type) +
                                  " exceeds TLV-VALUE boundary of parent block"));
    }
    // pos now points to TLV-VALUE of sub element

    Buffer::const_iterator subEnd = pos + length;
    m_elements.emplace_back(m_buffer, type, begin, subEnd, pos, subEnd);

    begin = subEnd;
  }
}

void Block::encode(){
  if (hasWire())
    return;

  EncodingEstimator estimator;
  size_t estimatedSize = encode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  encode(buffer);
}

size_t Block::encode(EncodingEstimator& estimator) const{
  if (hasValue()) {
    return m_size;
  }

  size_t len = encodeValue(estimator);
  len += estimator.prependVarNumber(len);
  len += estimator.prependVarNumber(m_type);
  return len;
}

size_t Block::encodeValue(EncodingEstimator& estimator) const{
  size_t len = 0;
  for (const Block& element : m_elements | boost::adaptors::reversed) {
    len += element.encode(estimator);
  }
  return len;
}

size_t Block::encode(EncodingBuffer& encoder){
  size_t len = 0;
  m_end = encoder.begin();
  if (hasValue()) {
    len += encoder.prependRange(m_valueBegin, m_valueEnd);
  }
  else {
    for (Block& element : m_elements | boost::adaptors::reversed) {
      len += element.encode(encoder);
    }
  }
  m_valueEnd = m_end;
  m_valueBegin = encoder.begin();

  len += encoder.prependVarNumber(len);
  len += encoder.prependVarNumber(m_type);
  m_begin = encoder.begin();

  m_buffer = encoder.getBuffer();
  m_size = len;
  return len;
}

const Block& Block::get(uint32_t type) const{
  auto it = this->find(type);
  if (it != m_elements.end()) {
    return *it;
  }

  BOOST_THROW_EXCEPTION(Error("No sub-element of type " + to_string(type) +
                              " is found in block of type " + to_string(m_type)));
}

Block::element_const_iterator Block::find(uint32_t type) const{
  return std::find_if(m_elements.begin(), m_elements.end(),
                      [type] (const Block& subBlock) { return subBlock.type() == type; });
}

void Block::remove(uint32_t type){
  resetWire();

  auto it = std::remove_if(m_elements.begin(), m_elements.end(),
                           [type] (const Block& subBlock) { return subBlock.type() == type; });
  m_elements.resize(it - m_elements.begin());
}

Block::element_iterator Block::erase(Block::element_const_iterator position){
  resetWire();
  return m_elements.erase(position);
}

Block::element_iterator Block::erase(Block::element_const_iterator first, Block::element_const_iterator last){
  resetWire();
  return m_elements.erase(first, last);
}

void Block::push_back(const Block& element){
  resetWire();
  m_elements.push_back(element);
}

Block::element_iterator Block::insert(Block::element_const_iterator pos, const Block& element){
  resetWire();
  return m_elements.insert(pos, element);
}
/******************************************sub elements***********************/

```
