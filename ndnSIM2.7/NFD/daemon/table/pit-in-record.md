/NFD/daemon/table/pit-in-record.hpp
```cpp
namespace nfd {
namespace pit {

// contains information about an Interest from an incoming face
// 包含来自输入Face的兴趣包相关信息
class InRecord : public FaceRecord
{
public:
  // 显式构造函数
  explicit InRecord(Face& face);

  // override重写
  void update(const Interest& interest);

  const Interest& getInterest() const;

private:
  shared_ptr<const Interest> m_interest;
};
/**********************************************************************/
inline const Interest& InRecord::getInterest() const{
  BOOST_ASSERT(static_cast<bool>(m_interest));
  return *m_interest;
}

} // namespace pit
} // namespace nfd
```

/NFD/daemon/table/pit-in-record.cpp
```cpp
namespace nfd {
namespace pit {

// 调用父类的构造函数---委托构造函数
InRecord::InRecord(Face& face)
  : FaceRecord(face){
}

void InRecord::update(const Interest& interest){
  // 调用父类的update
  this->FaceRecord::update(interest);
  // 去除interest的常量属性，并复制
  m_interest = const_cast<Interest&>(interest).shared_from_this();
}

} // namespace pit
} // namespace nfd
```
