NFD/daemon/table/cs-policy.hpp
```cpp
namespace nfd {
namespace cs {

class Cs;

// represents a CS replacement policy
class Policy : noncopyable
{
public: // registry
  template<typename P>
  static void registerPolicy(const std::string& policyName = P::POLICY_NAME){
    Registry& registry = getRegistry();
    // 断言当前注册的policy之前没有注册过
    BOOST_ASSERT(registry.count(policyName) == 0);
    registry[policyName] = [] { return make_unique<P>(); };
  }


  // 通过policyname创建cs::Policy；如果policyname未知则Policy为nullptr
  static unique_ptr<Policy> create(const std::string& policyName);


  // 返回存储可用策略名称的列表
  static std::set<std::string> getPolicyNames();

public:
  // 通过policyname显示构造Policy
  explicit Policy(const std::string& policyName);

  // 默认的虚构函数
  virtual  ~Policy() = default;

  const std::string& getName() const;

public:

  // gets cs
  Cs* getCs() const;

  // sets cs
  void setCs(Cs *cs);

  // gets hard limit (in number of entries)
  size_t getLimit() const;

  // sets hard limit (in number of entries)
  // getLimit() == nMaxEntries
  // cs.size() <= getLimit()
  // 必要情况下会执行evictEntries()驱逐entry
  void setLimit(size_t nMaxEntries);


  // 当一个entry被evict之前发出信号
  // 策略实现应该发出这个信号，并在信号发出时evict entry。
  signal::Signal<Policy, iterator> beforeEvict;


  // 在Cs_table中创建new entry之后调用该方法
  // 插入的时候会先检查空间是否足够，不够的话执行evict
  void afterInsert(iterator i);


  // 当一个已经存在的entry被相同的Data Packet刷新之后调用该方法
  void afterRefresh(iterator i);


  // 当一个entry由于管理命令被删除之前，该方法被Cs_policy调用
  // 如果一个entry由于空间不足被evict被erase则不会调用该方法
  void beforeErase(iterator i);


  // 当一个entry被用于匹配查找之前，该方法被Cs_policy调用
  void beforeUse(iterator i);

protected:
/************************************pure virtual function**********************************/
// 纯虚函数由subclass具体实现

  // 在Cs_table中创建new entry之后调用该方法
  // 当在子类中被重写时，策略实现应该决定是否接受i。
  // 如果i被接受，它应该被插入；否则，应该发出带有i的beforeEvict信号，通知CS驱逐条目。
  // 策略实现可以通过发出beforeEvict信号来决定驱逐其他条目，以保持CS的大小在限制之内。
  virtual void doAfterInsert(iterator i) = 0;


  // 当已经存在的entry被相同的数据包刷新之后调用该方法
  // When overridden in a subclass, a policy implementation may witness this operation and adjust its cleanup index.
  virtual void doAfterRefresh(iterator i) = 0;

  // 当一个entry由于管理命令被删除之前调用该方法；如果一个entry由于evict被erase则不会调用该方法
  // When overridden in a subclass, a policy implementation should erase from its cleanup index without emitted afterErase signal.
  virtual void doBeforeErase(iterator i) = 0;



  // 当Cs_table中的entry被用来匹配查找之前调用该方法
  // When overridden in a subclass, a policy implementation may witness this operation and adjust its cleanup index.
  virtual void doBeforeUse(iterator i) = 0;

  // 驱逐一个或者多个Entry直到Cs_size没有超出最大限制limit
  virtual void evictEntries() = 0;
/************************************pure virtual function**********************************/

protected:
  DECLARE_SIGNAL_EMIT(beforeEvict)

private: // registry
  typedef std::function<unique_ptr<Policy>()> CreateFunc;
  typedef std::map<std::string, CreateFunc> Registry; // indexed by policy name

  static Registry& getRegistry();

private:
  std::string m_policyName;//replace policy的名字
  size_t m_limit; //缓存容量
  Cs* m_cs;//Cs的实例：由一个[table:set]和一个policy组成
};
/***************************************************************************/
inline const std::string& Policy::getName() const{
  return m_policyName;
}

inline Cs* Policy::getCs() const{
  return m_cs;
}

inline void Policy::setCs(Cs *cs){
  m_cs = cs;
}

inline size_t Policy::getLimit() const{
  return m_limit;
}
/*********************************************************************************/

} // namespace cs
} // namespace nfd
```


NFD/daemon/table/cs-policy.cpp
```cpp
namespace nfd {
namespace cs {

Policy::Registry& Policy::getRegistry(){
  static Registry registry;
  return registry;
}

unique_ptr<Policy> Policy::create(const std::string& policyName){
  Registry& registry = getRegistry();
  // 执行 std::map::find
  auto i = registry.find(policyName);
  // policy name is unknow ： return nullptr
  return i == registry.end() ? nullptr : i->second();
}

std::set<std::string> Policy::getPolicyNames(){
  std::set<std::string> policyNames;
  boost::copy(getRegistry() | boost::adaptors::map_keys,
              std::inserter(policyNames, policyNames.end()));
  return policyNames;
}

Policy::Policy(const std::string& policyName): m_policyName(policyName){
}

void Policy::setLimit(size_t nMaxEntries){
  NFD_LOG_INFO("setLimit " << nMaxEntries);
  m_limit = nMaxEntries;
  this->evictEntries();
}

/*******************************************/
void Policy::afterInsert(iterator i){
  BOOST_ASSERT(m_cs != nullptr);
  this->doAfterInsert(i);
}

void Policy::afterRefresh(iterator i){
  BOOST_ASSERT(m_cs != nullptr);
  this->doAfterRefresh(i);
}

void Policy::beforeErase(iterator i){
  BOOST_ASSERT(m_cs != nullptr);
  this->doBeforeErase(i);
}

void Policy::beforeUse(iterator i){
  BOOST_ASSERT(m_cs != nullptr);
  this->doBeforeUse(i);
}
/************************************************/

} // namespace cs
} // namespace nfd
```
