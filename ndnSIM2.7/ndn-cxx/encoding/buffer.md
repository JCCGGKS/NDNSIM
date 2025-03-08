ndn-cxx/encoding/buffer.hpp
```cpp
namespace ndn {

/**
 * 通用的自动管理/调整大小的缓冲区
 * 在大多数情况下，Buffer类相当于' std::vector<uint8_t> '，实际上它使用后者作为基类。
 * 除此之外，它还提供了get<T>()方法，自动将返回的指针强制转换为所请求的类型。
 */
class Buffer : public std::vector<uint8_t>
{
public:

  // 默认构造函数
  Buffer() = default;

  // 拷贝构造函数
  Buffer(const Buffer&);

  // 拷贝赋值函数
  Buffer& operator=(const Buffer&);

  // 移动构造函数
  Buffer(Buffer&&) noexcept;

  // 移动赋值函数
  Buffer& operator=(Buffer&&) noexcept;

  // 创建特定大小的buffer
  // size： size of the Buffer to be allocated
  explicit Buffer(size_t size): std::vector<uint8_t>(size, 0){
  }


  // 通过从raw buffer中复制内容，创建Buffer
  // buf ：const pointer to buffer to copy
  // length ：length of the buffer to copy
  Buffer(const void* buf, size_t length)
    : std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(buf),
                           reinterpret_cast<const uint8_t*>(buf) + length){
  }


  // 通过复制特定范围[first,last)区间内的内容创建Buffer
  // first： an input iterator to the first element to copy
  // last： an input iterator to the element immediately following the last element to copy
  template<class InputIt>
  Buffer(InputIt first, InputIt last)
    : std::vector<uint8_t>(first, last){
  }

  // 返回指向buffer中第一个字节的指针，并强制转换为所请求的类型T
  template<class T>
  T* get() noexcept{
    return reinterpret_cast<T*>(data());
  }


  template<class T>
  const T* get() const noexcept{
    return reinterpret_cast<const T*>(data());
  }

};

/**************************************************************************/
// 拷贝构造函数
inline Buffer::Buffer(const Buffer&) = default;

// 拷贝赋值函数
inline Buffer& Buffer::operator=(const Buffer&) = default;

// 移动构造函数
inline Buffer::Buffer(Buffer&&) noexcept = default;

// 移动赋值函数
inline Buffer& Buffer::operator=(Buffer&&) noexcept = default;

using BufferPtr = shared_ptr<Buffer>;
using ConstBufferPtr = shared_ptr<const Buffer>;

} // namespace ndn
```
