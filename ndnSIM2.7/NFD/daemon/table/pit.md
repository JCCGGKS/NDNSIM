NFD/daemon/table/pit.hpp
```cpp
namespace nfd {
namespace pit {

/** \class nfd::pit::DataMatchResult
 *  \brief An unordered iterable of all PIT entries matching Data.
 *
 *  This type has the following member functions:
 *  - `iterator<shared_ptr<Entry>> begin()`
 *  - `iterator<shared_ptr<Entry>> end()`
 *  - `size_t size() const`
 */
using DataMatchResult = std::vector<shared_ptr<Entry>>;

// represents the Interest Table

class Pit : noncopyable
{
public:
  // 显示构造函数
  explicit Pit(NameTree& nameTree);


  // return number of entries
  size_t size() const{
    return m_nItems;
  }


  // 查找PIT entry
  // 返回和“Name、Selectors”相匹配的entry，不存在返回nullptr
  shared_ptr<Entry>
  find(const Interest& interest) const{
    return const_cast<Pit*>(this)->findOrInsert(interest, false).first;
  }


  // 插入一个PIT entry
  // interest ： the Interest; must be created with make_shared
  // 具有相同名称和选择器的new entry返回true，existing entry返回false
  std::pair<shared_ptr<Entry>, bool>
  insert(const Interest& interest){
    return this->findOrInsert(interest, true);
  }



  // 执行数据匹配操作
  // 返回所有与数据匹配的PIT条目的iterator对象
  DataMatchResult findAllDataMatches(const Data& data) const;

  // deletes an entry
  void erase(Entry* entry){
    this->erase(entry, true);
  }

  // deletes in-record and out-record for face
  void deleteInOutRecords(Entry* entry, const Face& face);

public: // enumeration
  typedef Iterator const_iterator;

  // return an iterator to the beginning
  // 迭代顺序是由实现定义的。
  // 如果在enumeration过程中插入或删除FIB/PIT/Measurements/StrategyChoice表项，则可能发生未定义行为。
  const_iterator begin() const;

  // return an iterator to the end
  const_iterator end() const{
    return Iterator();
  }

private:
  void erase(Entry* pitEntry, bool canDeleteNte);

  /** 找到或者插入一个PIT Entry
   *  \param
      interest ： the Interest; must be created with make_shared if allowInsert
   *  allowInsert ： whether inserting new entry is allowed.
   *  \return
      1.if allowInsert, a new or existing entry with same Name+Selectors,and true for new entry, false for existing entry;
   *  2.if not allowInsert, an existing entry with same Name+Selectors and false,or {nullptr, true} if there's no existing entry
   */
  std::pair<shared_ptr<Entry>, bool>
  findOrInsert(const Interest& interest, bool allowInsert);

private:
  // NameTree的声明在/NFD/daemon/table/name-tree.hpp
  NameTree& m_nameTree;
  // entry的个数
  size_t m_nItems;
};

} // namespace pit

using pit::Pit;

} // namespace nfd
```


NFD/daemon/table/pit.cpp
```cpp
namespace nfd {
namespace pit {

static inline bool nteHasPitEntries(const name_tree::Entry& nte){
  return nte.hasPitEntries();
}

Pit::Pit(NameTree& nameTree)
  : m_nameTree(nameTree)
  , m_nItems(0){
}

/*****************************findOrInsert**************************************/
std::pair<shared_ptr<Entry>, bool>
Pit::findOrInsert(const Interest& interest, bool allowInsert){
  // determine which NameTree entry should the PIT entry be attached onto
  const Name& name = interest.getName();
  bool hasDigest = name.size() > 0 && name[-1].isImplicitSha256Digest();
  size_t nteDepth = name.size() - static_cast<int>(hasDigest);
  nteDepth = std::min(nteDepth, NameTree::getMaxDepth());

  // ensure NameTree entry exists
  name_tree::Entry* nte = nullptr;
  if (allowInsert) {
    nte = &m_nameTree.lookup(name, nteDepth);
  }
  else {
    nte = m_nameTree.findExactMatch(name, nteDepth);
    if (nte == nullptr) {
      return {nullptr, true};
    }
  }

  // check if PIT entry already exists
  const auto& pitEntries = nte->getPitEntries();
  auto it = std::find_if(pitEntries.begin(), pitEntries.end(),
    [&interest, nteDepth] (const shared_ptr<Entry>& entry) {
      // NameTree guarantees first nteDepth components are equal
      return entry->canMatch(interest, nteDepth);
    });
  if (it != pitEntries.end()) {
    return {*it, false};
  }

  if (!allowInsert) {
    BOOST_ASSERT(!nte->isEmpty()); // nte shouldn't be created in this call
    return {nullptr, true};
  }

  auto entry = make_shared<Entry>(interest);
  nte->insertPitEntry(entry);
  ++m_nItems;
  return {entry, true};
}

//using DataMatchResult = std::vector<shared_ptr<Entry>>;
// 查找所有请求该数据包的PIT entry
DataMatchResult Pit::findAllDataMatches(const Data& data) const{
  // 右值引用
  // findAllMatches声明在/NFD/daemon/table/name-tree.hpp
  auto&& ntMatches = m_nameTree.findAllMatches(data.getName(), &nteHasPitEntries);

  DataMatchResult matches;
  for (const name_tree::Entry& nte : ntMatches) {
    for (const shared_ptr<Entry>& pitEntry : nte.getPitEntries()) {
      if (pitEntry->getInterest().matchesData(data))
        matches.emplace_back(pitEntry);
    }
  }

  return matches;
}
/*****************************findOrInsert**************************************/


/*****************************erase********************************/
void Pit::erase(Entry* entry, bool canDeleteNte){
  name_tree::Entry* nte = m_nameTree.getEntry(*entry);
  BOOST_ASSERT(nte != nullptr);

  nte->erasePitEntry(entry);
  if (canDeleteNte) {
    m_nameTree.eraseIfEmpty(nte);
  }
  --m_nItems;
}
/*****************************erase********************************/


void Pit::deleteInOutRecords(Entry* entry, const Face& face){
  BOOST_ASSERT(entry != nullptr);

  entry->deleteInRecord(face);
  entry->deleteOutRecord(face);

  // 如果没有in/out-record，决定是否删除PIT entry
}


Pit::const_iterator Pit::begin() const{
  return const_iterator(m_nameTree.fullEnumerate(&nteHasPitEntries).begin());
}

} // namespace pit
} // namespace nfd
```
