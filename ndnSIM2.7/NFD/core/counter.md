/NFD/core/counter.hpp
```cpp
namespace nfd {

// 表示包含整数值的计数器
/*  SimpleCounter is noncopyable, because increment should be called on the counter,
 *  not a copy of it; it's implicitly convertible to an integral type to be observed
 */
/*************************************SimpleCounter*****************************/
class SimpleCounter
{
public:
  typedef uint64_t rep;

  // 常量构造函数
  constexpr SimpleCounter(): m_value(0){
  }

  // 不允许复制，删除拷贝构造函数
  SimpleCounter(const SimpleCounter&) = delete;
  // 不允许复制，删除拷贝赋值函数
  SimpleCounter& operator=(const SimpleCounter&) = delete;


  // 隐式类型转换：从类类型转换为rep类型
  // SimpleCounter sc; uint64_t val= sc;/*val=0*/
  operator rep() const{
    return m_value;
  }

  // replace the counter value
  void set(rep value){
    m_value = value;
  }

protected:
  rep m_value;
};
/*************************************SimpleCounter*****************************/

/** represents a counter of number of packets
 *  \warning The counter value may wrap after exceeding the range of underlying integer type.
 */
/*************************************PacketCounter*****************************/
class PacketCounter : public SimpleCounter
{
public:

  // increment the counter by one
  // prefix++,返回自增之后的对象
  PacketCounter& operator++(){
    ++m_value;
    return *this;
  }
  // postfix ++ operator is not provided because it's not needed
};
/*************************************PacketCounter*****************************/


/** represents a counter of number of bytes
 *  \warning The counter value may wrap after exceeding the range of underlying integer type.
 */
/*************************************ByteCounter********************************/
class ByteCounter : public SimpleCounter
{
public:

  // increase the counter
  // prefix++
  ByteCounter& operator+=(rep n){
    m_value += n;
    return *this;
  }
};
/*************************************ByteCounter********************************/


/*  provides a counter that observes the size of a table
 *  \tparam T  ： a type that provides a size() const member function
 *  if table not specified in constructor, it can be added later by invoking observe()
 */
/*************************************SizeCounter***********************************/
template<typename T>
class SizeCounter
{
public:
  typedef size_t Rep;

  explicit constexpr SizeCounter(const T* table = nullptr): m_table(table){
  }

  // 不允许复制，拷贝构造函数被删除
  SizeCounter(const SizeCounter&) = delete;
  // 不允许复制，拷贝赋值函数被删除
  SizeCounter& operator=(const SizeCounter&) = delete;


  void observe(const T* table){
    m_table = table;
  }

  // observe the counter
  operator Rep() const{
    BOOST_ASSERT(m_table != nullptr);
    return m_table->size();
  }

private:
  const T* m_table;
};
/*************************************SizeCounter********************************/
} // namespace nfd
```
