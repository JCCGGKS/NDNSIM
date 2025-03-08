/NFD/daemon/fw/retx-suppression.hpp
用来标志一个兴趣包的传输是否受到了抑制
```cpp
namespace nfd {
namespace fw {

// enum class是限定作用域的枚举类型，详见/C++/enum_class.md
enum class RetxSuppressionResult {

  // Interest is new (not a retransmission)
  // 兴趣是新的(不是重传)
  NEW,

  // Interest is retransmission and should be forwarded
  // 兴趣是重传，应该转发
  FORWARD,

  // Interest is retransmission and should be suppressed
  // 兴趣重传，应该被抑制
  SUPPRESS
};

} // namespace fw
} // namespace nfd
```
