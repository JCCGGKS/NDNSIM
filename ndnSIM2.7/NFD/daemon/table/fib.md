/NFD/daemon/table/fib.hpp
```cpp
namespace nfd {

namespace measurements {
class Entry;
} // namespace measurements
namespace pit {
class Entry;
} // namespace pit

namespace fib {

// represents the Forwarding Information Base (FIB)
class Fib : noncopyable
{
public:
  // 显示构造函数
  explicit Fib(NameTree& nameTree);

  // 获取表中entry的个数
  size_t size() const{
    return m_nItems;
  }

public: // lookup

/***************************************lookup***********************************/

  // performs a longest prefix match
  const Entry& findLongestPrefixMatch(const Name& prefix) const;


  // performs a longest prefix match
  // This is equivalent to .findLongestPrefixMatch(pitEntry.getName())
  const Entry& findLongestPrefixMatch(const pit::Entry& pitEntry) const;


  // performs a longest prefix match
  // This is equivalent to .findLongestPrefixMatch(measurementsEntry.getName())
  const Entry& findLongestPrefixMatch(const measurements::Entry& measurementsEntry) const;


  // performs an exact match lookup
  Entry* findExactMatch(const Name& prefix);
/***************************************lookup***********************************/


public: // mutation

  // Maximum number of components in a FIB entry prefix.
  // FIB Entry名称前缀的最大组件个数，声明在/NFD/core/fib-max-depth.hpp
  /*static const int FIB_MAX_DEPTH = 32;*/
  static constexpr size_t getMaxDepth(){
    return FIB_MAX_DEPTH;
  }

  // find or insert a FIB entry
  // prefix ： FIB entry name; it must have no more than getMaxDepth() components.
  // return the entry, and true for new entry or false for existing entry
  std::pair<Entry*, bool> insert(const Name& prefix);

  void erase(const Name& prefix);


  void erase(const Entry& entry);


  // removes the NextHop record for face with a given endpointId
  void removeNextHop(Entry& entry, const Face& face, uint64_t endpointId);


  // removes the NextHop record for face for any endpointId
  void removeNextHopByFace(Entry& entry, const Face& face);

public: // enumeration
  typedef boost::transformed_range<name_tree::GetTableEntry<Entry>, const name_tree::Range> Range;
  typedef boost::range_iterator<Range>::type const_iterator;

  /** return an iterator to the beginning
   *  迭代顺序是由实现定义的。
   *  如果在枚举过程中插入或删除FIB/PIT/Measurements/StrategyChoice表项，则可能发生未定义行为。
   */
  const_iterator begin() const{
    return this->getRange().begin();
  }

  // return an iterator to the end
  const_iterator end() const{
    return this->getRange().end();
  }

private:

  // \tparam K a parameter acceptable to NameTree::findLongestPrefixMatch
  template<typename K>
  const Entry& findLongestPrefixMatchImpl(const K& key) const;


  void erase(name_tree::Entry* nte, bool canDeleteNte = true);


  // erase entry if it contains no nexthop record
  void eraseIfEmpty(Entry& entry);

  //
  Range getRange() const;

private:
  NameTree& m_nameTree;
  size_t m_nItems;


  /** the empty FIB entry.
   *  This entry has no nexthops.
   *  It is returned by findLongestPrefixMatch if nothing is matched.
   */
  static const unique_ptr<Entry> s_emptyEntry;
};

} // namespace fib

using fib::Fib;

} // namespace nfd
```


/NFD/daemon/table/fib.cpp
```cpp
namespace nfd {
namespace fib {

NDN_CXX_ASSERT_FORWARD_ITERATOR(Fib::const_iterator);

// static成员变量类外初始化
const unique_ptr<Entry> Fib::s_emptyEntry = make_unique<Entry>(Name());

static inline bool nteHasFibEntry(const name_tree::Entry& nte){
  return nte.getFibEntry() != nullptr;
}

Fib::Fib(NameTree& nameTree)
  : m_nameTree(nameTree)
  , m_nItems(0){
}

template<typename K>
const Entry& Fib::findLongestPrefixMatchImpl(const K& key) const{
  name_tree::Entry* nte = m_nameTree.findLongestPrefixMatch(key, &nteHasFibEntry);
  // 找到返回Fib entry
  if (nte != nullptr) {
    return *nte->getFibEntry();
  }
  // 找不到返回s_emptyEntry
  return *s_emptyEntry;
}


const Entry& Fib::findLongestPrefixMatch(const Name& prefix) const{
  return this->findLongestPrefixMatchImpl(prefix);
}

const Entry&
Fib::findLongestPrefixMatch(const pit::Entry& pitEntry) const
{
  return this->findLongestPrefixMatchImpl(pitEntry);
}

const Entry&
Fib::findLongestPrefixMatch(const measurements::Entry& measurementsEntry) const
{
  return this->findLongestPrefixMatchImpl(measurementsEntry);
}

Entry* Fib::findExactMatch(const Name& prefix){
  name_tree::Entry* nte = m_nameTree.findExactMatch(prefix);
  if (nte != nullptr)
    return nte->getFibEntry();

  return nullptr;
}

std::pair<Entry*, bool> Fib::insert(const Name& prefix){
  name_tree::Entry& nte = m_nameTree.lookup(prefix);
  Entry* entry = nte.getFibEntry();
  // 找到返回
  if (entry != nullptr) {
    return {entry, false};
  }

  // 找不到插入
  nte.setFibEntry(make_unique<Entry>(prefix));
  ++m_nItems;
  return {nte.getFibEntry(), true};
}

void Fib::erase(name_tree::Entry* nte, bool canDeleteNte){
  BOOST_ASSERT(nte != nullptr);

  nte->setFibEntry(nullptr);
  if (canDeleteNte) {
    m_nameTree.eraseIfEmpty(nte);
  }
  --m_nItems;
}

void Fib::erase(const Name& prefix){
  name_tree::Entry* nte = m_nameTree.findExactMatch(prefix);
  if (nte != nullptr) {
    // 调用Fib::erase(name_tree::Entry* nte, bool canDeleteNte)
    this->erase(nte);
  }
}

void Fib::erase(const Entry& entry){
  name_tree::Entry* nte = m_nameTree.getEntry(entry);
  if (nte == nullptr) { // don't try to erase s_emptyEntry
    BOOST_ASSERT(&entry == s_emptyEntry.get());
    return;
  }
  // 调用Fib::erase(name_tree::Entry* nte, bool canDeleteNte)
  this->erase(nte);
}

void Fib::eraseIfEmpty(Entry& entry){
  if (!entry.hasNextHops()) {
    name_tree::Entry* nte = m_nameTree.getEntry(entry);
    // 调用Fib::erase(name_tree::Entry* nte, bool canDeleteNte)
    this->erase(nte, false);
  }
}

void Fib::removeNextHop(Entry& entry, const Face& face, uint64_t endpointId){
  entry.removeNextHop(face, endpointId);
  this->eraseIfEmpty(entry);
}

void Fib::removeNextHopByFace(Entry& entry, const Face& face){
  entry.removeNextHopByFace(face);
  this->eraseIfEmpty(entry);
}

Fib::Range Fib::getRange() const{
  return m_nameTree.fullEnumerate(&nteHasFibEntry) |
         boost::adaptors::transformed(name_tree::GetTableEntry<Entry>(&name_tree::Entry::getFibEntry));
}

} // namespace fib
} // namespace nfd
```
