NFD/daemon/table/cs.hpp
```cpp
namespace nfd {
namespace cs {

/** Content Store的实现
* CS的实现由一个表(Table)和一个替换策略(replacement policy)组成。
* table是一个容器(std::set)，按照存储的Data Packet的full name进行排序。**[没看代码之前我一直以为CS是由map实现的]**
* Data Packet被封装在Entry对象中。每个Entry包含Data Packet本身，以及一些附加属性，例如数据何时变为非新鲜的。
* replacemnet policy在policy的子类中实现。
*/
class Cs : noncopyable
{
public:
  // 显示构造函数
  explicit Cs(size_t nMaxPackets = 10);

  // inserts a Data packet
  void insert(const Data& data, bool isUnsolicited = false);

  using AfterEraseCallback = std::function<void(size_t nErased)>;

  /** asynchronously erases entries under prefix
      异步擦除prefix下的entries
   *  \param
      prefix : name prefix of entries
   *  limit : max number of entries to erase
   *  cb : callback to receive the actual number of erased entries; it may be empty;
   *            it may be invoked either before or after erase() returns
   */
  void erase(const Name& prefix, size_t limit, const AfterEraseCallback& cb);

  using HitCallback = std::function<void(const Interest&, const Data&)>;
  using MissCallback = std::function<void(const Interest&)>;

  /** 找到最匹配的Data Packet
   *  \param
      interest ： the Interest for lookup
   *  hitCallback ： a callback if a match is found; must not be empty
   *  missCallback ： a callback if there's no match; must not be empty
   * /
  // 一次lookup只能执行一次callback；callback的执行可以在find之前或者之后
  void find(const Interest& interest,
       const HitCallback& hitCallback,
       const MissCallback& missCallback) const;


  // get number of stored packets
  size_t size() const{
    return m_table.size();
  }

public: // configuration

  // get capacity (in number of packets)
  size_t getLimit() const{
    return m_policy->getLimit();
  }

  // change capacity (in number of packets)
  void setLimit(size_t nMaxPackets){
    return m_policy->setLimit(nMaxPackets);
  }

  // get replacement policy
  Policy* getPolicy() const{
    return m_policy.get();
  }

  // change replacement policy
  // \pre size() == 0
  void setPolicy(unique_ptr<Policy> policy);

  // get CS_ENABLE_ADMIT flag
  // \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
  bool shouldAdmit() const{
    return m_shouldAdmit;
  }

  // set CS_ENABLE_ADMIT flag
  // \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
  void enableAdmit(bool shouldAdmit);

  // get CS_ENABLE_SERVE flag
  // \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
  bool shouldServe() const{
    return m_shouldServe;
  }

  // set CS_ENABLE_SERVE flag
  // https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
  void enableServe(bool shouldServe);

public: // enumeration
  struct EntryFromEntryImpl
  {
    typedef const Entry& result_type;

    /*
    class EntryImpl : public Entry{...}
    将EntryImpl 隐式转换为 Entry：向上转换
    */
    const Entry& operator()(const EntryImpl& entry) const{
      return entry;
    }
  };

/***************************ContentStore iterator (public API)***********************************/
  typedef boost::transform_iterator<EntryFromEntryImpl, iterator, const Entry&> const_iterator;

  const_iterator begin() const{
    return boost::make_transform_iterator(m_table.begin(), EntryFromEntryImpl());
  }

  const_iterator end() const{
    return boost::make_transform_iterator(m_table.end(), EntryFromEntryImpl());
  }

private: // find

  // find leftmost match in [first,last)
  // return the leftmost match, or last if not found
  iterator findLeftmost(const Interest& interest, iterator left, iterator right) const;

  // find rightmost match in [first,last)
  // return the rightmost match, or last if not found
  iterator findRightmost(const Interest& interest, iterator first, iterator last) const;

  // find rightmost match among entries with exact Names in [first,last)
  // return the rightmost match, or last if not found
  iterator findRightmostAmongExact(const Interest& interest, iterator first, iterator last) const;

  void setPolicyImpl(unique_ptr<Policy> policy);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void dump();

private:
  // 这里的Table声明在NFD/daemon/table/cs-internal.hpp
  Table m_table;
  // 这里的Policy声明在NFD/daemon/table/cs-policy.hpp
  unique_ptr<Policy> m_policy;
  signal::ScopedConnection m_beforeEvictConnection;

  bool m_shouldAdmit; //指示Data Packet是否可以被cache if false, no Data will be admitted
  bool m_shouldServe; //指示CS表是否工作， if false, all lookups will miss
};

} // namespace cs

using cs::Cs;

} // namespace nfd
```
NFD/daemon/table/cs.cpp
```cpp
namespace nfd {
namespace cs {

NDN_CXX_ASSERT_FORWARD_ITERATOR(Cs::const_iterator);

NFD_LOG_INIT(ContentStore);

static unique_ptr<Policy> makeDefaultPolicy(){
  // 默认的replacement policy是lru
  return Policy::create("lru");
}

Cs::Cs(size_t nMaxPackets)
  : m_shouldAdmit(true)
  , m_shouldServe(true){
  this->setPolicyImpl(makeDefaultPolicy());
  m_policy->setLimit(nMaxPackets);
}

void Cs::insert(const Data& data, bool isUnsolicited){

  // 不允许Data Packet进入，或者缓存空间为0；直接退出
  if (!m_shouldAdmit || m_policy->getLimit() == 0) {
    return;
  }
  NFD_LOG_DEBUG("insert " << data.getName());

  // recognize CachePolicy
  // 获取Data Packet的CachePolicyTag
  shared_ptr<lp::CachePolicyTag> tag = data.getTag<lp::CachePolicyTag>();
  if (tag != nullptr) {
    // 获取tag中记录的cache policy
    lp::CachePolicyType policy = tag->get().getPolicy();

    // 如果缓存策略为NO_CACHE即不缓存，直接退出
    if (policy == lp::CachePolicyType::NO_CACHE) {
      return;
    }
  }

  // 执行插入操作
  iterator it;
  bool isNewEntry = false;
  /* set::emplace的返回值：pair<iterator,bool>；
  * bool为true指示新元素插入成功，iterator指向新插入元素的位置
  * bool为false指示插入的元素已经存在，iterator指向原有元素的位置*/
  //
  std::tie(it, isNewEntry) = m_table.emplace(data.shared_from_this(), isUnsolicited);
  // typedef Table::const_iterator iterator;
  // 用const_cast去掉const标识
  EntryImpl& entry = const_cast<EntryImpl&>(*it);

  entry.updateStaleTime();

  if (!isNewEntry) { // existing entry
    // XXX 这并不禁止未经请求的数据刷新请求的条目。
    if (entry.isUnsolicited() && !isUnsolicited) {
      entry.unsetUnsolicited();
    }

    m_policy->afterRefresh(it);
  }
  else {
    m_policy->afterInsert(it);
  }
}


void Cs::erase(const Name& prefix, size_t limit, const AfterEraseCallback& cb){
  BOOST_ASSERT(static_cast<bool>(cb));

  iterator first = m_table.lower_bound(prefix);
  iterator last = m_table.end();
  if (prefix.size() > 0) {
    last = m_table.lower_bound(prefix.getSuccessor());
  }

  size_t nErased = 0;
  while (first != last && nErased < limit) {
    m_policy->beforeErase(first);
    // 返回指向最后一个被删除的元素后面的元素
    first = m_table.erase(first);
    ++nErased;
  }

  // 此时nErased是有效删除的元素数量
  if (cb) {
    cb(nErased);
  }
}

void Cs::find(const Interest& interest,
         const HitCallback& hitCallback,
         const MissCallback& missCallback) const
{
  BOOST_ASSERT(static_cast<bool>(hitCallback));
  BOOST_ASSERT(static_cast<bool>(missCallback));

  //CS表被禁用，或者CS表的空间为0
  if (!m_shouldServe || m_policy->getLimit() == 0) {
    missCallback(interest);
    return;
  }
  // 获取interest packet的前缀名
  const Name& prefix = interest.getName();

  bool isRightmost = interest.getChildSelector() == 1;
  NFD_LOG_DEBUG("find " << prefix << (isRightmost ? " R" : " L"));

  // set::lower_bound：返回容器内第一次大于等于prefix的迭代器
  // set::upper_bound：返回容器内第一次大于prefix的迭代器
  // 因为set本身是有序的，所以可以使用binary_search实现lower_bound和upper_bound
  iterator first = m_table.lower_bound(prefix);
  iterator last = m_table.end();
  if (prefix.size() > 0) {
    last = m_table.lower_bound(prefix.getSuccessor());
  }

  iterator match = last;
  if (isRightmost) {
    match = this->findRightmost(interest, first, last);
  }
  else {
    match = this->findLeftmost(interest, first, last);
  }

  // cache miss
  if (match == last) {
    NFD_LOG_DEBUG("  no-match");
    missCallback(interest);
    return;
  }
  // cache hit
  NFD_LOG_DEBUG("  matching " << match->getName());
  m_policy->beforeUse(match);
  hitCallback(interest, match->getData());
}

iterator
Cs::findLeftmost(const Interest& interest, iterator first, iterator last) const{
  // find_if：从前往后找，找到[first,last)区间内第一次满足条件的iterator
  return std::find_if(first, last, [&interest] (const auto& entry) { return entry.canSatisfy(interest); });
}

iterator
Cs::findRightmost(const Interest& interest, iterator first, iterator last) const{
  // Each loop visits a sub-namespace under a prefix one component longer than Interest Name.
  // If there is a match in that sub-namespace, the leftmost match is returned;
  // otherwise, loop continues.

  size_t interestNameLength = interest.getName().size();
  for (iterator right = last; right != first;) {
    iterator prev = std::prev(right);

    // special case: [first,prev] have exact Names
    if (prev->getName().size() == interestNameLength) {
      NFD_LOG_TRACE("  find-among-exact " << prev->getName());
      iterator matchExact = this->findRightmostAmongExact(interest, first, right);
      return matchExact == right ? last : matchExact;
    }

    Name prefix = prev->getName().getPrefix(interestNameLength + 1);
    iterator left = m_table.lower_bound(prefix);

    // normal case: [left,right) are under one-component-longer prefix
    NFD_LOG_TRACE("  find-under-prefix " << prefix);
    iterator match = this->findLeftmost(interest, left, right);
    if (match != right) {
      return match;
    }
    right = left;
  }
  return last;
}

iterator
Cs::findRightmostAmongExact(const Interest& interest, iterator first, iterator last) const{
  // find_last_if，定义在/NFD/core/algorithm.hpp
  return find_last_if(first, last, [&interest] (const auto& entry) { return entry.canSatisfy(interest); });
}

void Cs::dump(){
  NFD_LOG_DEBUG("dump table");
  for (const EntryImpl& entry : m_table) {
    NFD_LOG_TRACE(entry.getFullName());
  }
}

void Cs::setPolicy(unique_ptr<Policy> policy){
  BOOST_ASSERT(policy != nullptr);
  BOOST_ASSERT(m_policy != nullptr);
  //
  size_t limit = m_policy->getLimit();
  this->setPolicyImpl(std::move(policy));
  m_policy->setLimit(limit);
}

void Cs::setPolicyImpl(unique_ptr<Policy> policy){
  NFD_LOG_DEBUG("set-policy " << policy->getName());
  m_policy = std::move(policy);
  // connect里面的参数是一个lambda匿名函数
  m_beforeEvictConnection = m_policy->beforeEvict.connect([this] (iterator it) {
      m_table.erase(it);
    });

  m_policy->setCs(this);
  // 断言是否设置成功
  BOOST_ASSERT(m_policy->getCs() == this);
}

void Cs::enableAdmit(bool shouldAdmit){
  if (m_shouldAdmit == shouldAdmit) {
    return;
  }
  m_shouldAdmit = shouldAdmit;
  NFD_LOG_INFO((shouldAdmit ? "Enabling" : "Disabling") << " Data admittance");
}

void Cs::enableServe(bool shouldServe){
  // 设置之前先检查
  if (m_shouldServe == shouldServe) {
    return;
  }
  m_shouldServe = shouldServe;
  NFD_LOG_INFO((shouldServe ? "Enabling" : "Disabling") << " Data serving");
}

} // namespace cs
} // namespace nfd
```
