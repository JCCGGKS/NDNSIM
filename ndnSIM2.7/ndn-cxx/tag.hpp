ndn-cxx/tag.hpp

ndn-cxx/tag.hpp
```cpp
// ndn-cxx/tag.hpp
namespace ndn {
// 可以保存任意信息的包标签的基类
class Tag
{
public:
  virtual ~Tag();

// 返回唯一标识此标记类型的整数
//https://redmine.named-data.net/projects/ndn-cxx/wiki/PacketTagTypes
// 当在一个函数的声明或定义末尾使用noexcept时，告诉编译器这个函数保证不会抛出任何异常。
// 如果该函数实际上抛出了异常，程序将调用std::terminate，通常会导致程序终止。
#ifdef DOXYGEN
  static constexpr int getTypeId() noexcept{
    return <type-identifier>;
  }
#endif
};

inline Tag::~Tag() = default;

// Brief为简单类型提供了一个标记类型
// 参数T为值类型
template<typename T, int TypeId>
class SimpleTag : public Tag
{
public:
  static constexpr int getTypeId() noexcept{
    return TypeId;
  }

  // 从T显式转换
  constexpr explicit SimpleTag(const T& value) noexcept : m_value(value){
  }

  // 隐式转换为T，返回所包含值的副本
  operator T() const{
    return m_value;
  }

  // 返回所包含的值
  constexpr const T& get() const noexcept{
    return m_value;
  }

private:
  T m_value;
};

} // namespace ndn
```
