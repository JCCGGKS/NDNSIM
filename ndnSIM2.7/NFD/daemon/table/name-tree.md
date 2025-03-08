/NFD/daemon/table/name-tree.hpp 
```cpp
namespace nfd {
namespace name_tree {

/**索引结构 
	a common index structure for FIB, PIT, StrategyChoice, and Measurements
 */
class NameTree : noncopyable
{
public:
  // 显示构造函数
  explicit NameTree(size_t nBuckets = 1024);

public: // information
  /** name tree的最大深度
   *  如果有一个包含许多componets的name调用NameTree::lookup，会导致创建许多name tree entry，这回耗费许多时间
   *  这个常量限制了NameTree条目名称中名称组件的最大数量。
   *  因此，它限制了从长名称创建的NameTree条目的数量，从而限制了处理的复杂性。
   */
  static constexpr size_t getMaxDepth(){
    // 定义在NFD/core/fib-max-depth.hpp
    // FIB表项前缀的最大组件数。
    // static const int FIB_MAX_DEPTH = 32;
    return FIB_MAX_DEPTH;
  }

  // return number of name tree entries
  size_t size() const{
    return m_ht.size();
  }

  // return number of hashtable buckets
  size_t getNBuckets() const{
    return m_ht.getNBuckets();
  }

  // return name tree entry on which a table entry is attached,or nullptr if the table entry is detached
  // 返回name tree entry，如果该表项已经被分离则返回nullptr
  template<typename EntryT>
  Entry* getEntry(const EntryT& tableEntry) const{
    return Entry::get(tableEntry);
  }

public: // mutation
  /** 按照name查找或者插入entry
   *
   *  此方法查找名称为name.getprefix(prefixLen)的名称树条目。
   *  如果条目不存在，则与所有祖先条目一起创建。
   *  在此操作期间，现有迭代器不受影响。
   *
   *  \warning prefixLen must not exceed name.size().
   *  \warning prefixLen must not exceed getMaxDepth().
   */
  Entry& lookup(const Name& name, size_t prefixLen);

  // 等效于 `lookup(name, name.size())`
  Entry& lookup(const Name& name){
    return this->lookup(name, name.size());
  }

  /**  等效于`lookup(fibEntry.getPrefix())`
   *  \param fibEntry ： a FIB entry attached to this name tree, or Fib::s_emptyEntry
   *  在普遍情况下，该重载比 `lookup(const Name&)` 更有效。
   */
  Entry& lookup(const fib::Entry& fibEntry);

  /** 等效于lookup(pitEntry.getName(), std::min(pitEntry.getName().size(), getMaxDepth()))`
   *  \param pitEntry ： a PIT entry attached to this name tree
   *  在普遍情况下，这个重载比`lookup(const Name&)`更有效。
   */
  Entry& lookup(const pit::Entry& pitEntry);


  /** 等效于`lookup(measurementsEntry.getName())`
   *  \param measurementsEntry ： a Measurements entry attached to this name tree
   *  在普遍情况下，这个重载比`lookup(const Name&)`更有效。
   */
  Entry& lookup(const measurements::Entry& measurementsEntry);

  /** 等效于`lookup(strategyChoiceEntry.getPrefix())`
   *  \param strategyChoiceEntry ： a StrategyChoice entry attached to this name tree
   *  在普遍情况下，这个重载比`lookup(const Name&)`更有效。
   */
  Entry& lookup(const strategy_choice::Entry& strategyChoiceEntry);


  /** 删除entry如果它是空的
   *  \param
      entry  ： a valid entry
   *  canEraseAncestors  ： whether ancestors should be deleted if they become empty
   *  返回删除entries的数量，Entry::isEmpty()

   *  如果entry为空，就会被删除。如果 “canEraseAncestors ”为 true，entry的祖先也会被删除。
      此函数必须在从现有迭代器中分离表项后调用，但指向已删除项的迭代器不受影响。
   */
  size_t eraseIfEmpty(Entry* entry, bool canEraseAncestors = true);

public: // matching

  // 精确匹配查找
  // 返回带有name.getPrefix(prefixLen)的entry，如果不存在则返回nullptr
  Entry* findExactMatch(const Name& name, size_t prefixLen = std::numeric_limits<size_t>::max()) const;

  /** 最长前缀匹配
   *  "返回一个名称是'name'前缀的entry，该entry通过了`entrySelector`的筛选，且没有更长名称的entry满足这些要求；
   *  如果不存在满足这些要求的entry，则返回空指针（nullptr）。"
   */
  Entry* findLongestPrefixMatch(const Name& name,
                         const EntrySelector& entrySelector = AnyEntry()) const;

  /** 等效于 `findLongestPrefixMatch(entry.getName(), entrySelector)`
   在一般情况下，这个重载比findLongestPrefixMatch(const Name&， const EntrySelector&)更有效。
   */
  Entry* findLongestPrefixMatch(const Entry& entry,
                         const EntrySelector& entrySelector = AnyEntry()) const;

  /** 等效于`findLongestPrefixMatch(getEntry(tableEntry)->getName(), entrySelector)`
   *  \tparam EntryT ： fib::Entry or measurements::Entry or strategy_choice::Entry
   *  在一般情况下，这个重载比findLongestPrefixMatch(const Name&， const EntrySelector&)更有效。
   *  如果tableEntry没有附加到这个名称树，可能会发生未定义的行为。
   */
  template<typename EntryT>
  Entry* findLongestPrefixMatch(const EntryT& tableEntry,
                         const EntrySelector& entrySelector = AnyEntry()) const{
    const Entry* nte = this->getEntry(tableEntry);
    BOOST_ASSERT(nte != nullptr);
    return this->findLongestPrefixMatch(*nte, entrySelector);
  }

  /** 等效于`findLongestPrefixMatch(pitEntry.getName(), entrySelector)`
   * 在一般情况下，这个重载比findLongestPrefixMatch(const Name&， const EntrySelector&)更有效。
   * 如果pitEntry没有附加到这个名称树，可能会发生未定义的行为。
   */
  Entry*
  findLongestPrefixMatch(const pit::Entry& pitEntry,
                         const EntrySelector& entrySelector = AnyEntry()) const;

  /** all-prefixes match lookup
   *  \return a range where every entry has a name that is a prefix of name ,and matches entrySelector.
   *
   *  Example:
   *  \code
   *  Name name("/A/B/C");
   *  auto&& allMatches = nt.findAllMatches(name);
   *  for (const Entry& nte : allMatches) {
   *    BOOST_ASSERT(nte.getName().isPrefixOf(name));
   *    ...
   *  }
   *  \endcode
   *  \note Iteration order is implementation-defined.
   *  \warning If a name tree entry whose name is a prefix of \p name is inserted
   *           during the enumeration, it may or may not be visited.
   *           If a name tree entry whose name is a prefix of \p name is deleted
   *           during the enumeration, undefined behavior may occur.
   */
  Range
  findAllMatches(const Name& name,
                 const EntrySelector& entrySelector = AnyEntry()) const;

public: // enumeration
  using const_iterator = Iterator;

  /** 枚举所有entries
   *  返回一个range， 每一个和entrySelector匹配的entry
   *
   *  Example:
   *  \code
   *  auto&& enumerable = nt.fullEnumerate();
   *  for (const Entry& nte : enumerable) {
   *    ...
   *  }
   *  \endcode
   *  迭代顺序是由实现定义的。
   *  如果在枚举过程中插入或删除\p prefix下的名称树项，可能会导致枚举跳过项或访问某些项两次。
   */
  Range fullEnumerate(const EntrySelector& entrySelector = AnyEntry()) const;

  /** 枚举prefix下的所有项
   *  返回一个range，其中每个entry都有一个以prefix开头的名称，并与entrySubTreeSelector匹配。
   *
   *  Example:
   *  \code
   *  Name name("/A/B/C");
   *  auto&& enumerable = nt.partialEnumerate(name);
   *  for (const Entry& nte : enumerable) {
   *    BOOST_ASSERT(name.isPrefixOf(nte.getName()));
   *    ...
   *  }
   *  \endcode

   *迭代顺序是由实现定义的。
   *如果在枚举过程中插入或删除prefix下的名称树项，可能会导致枚举跳过项或访问某些项两次。
   */
  // using Range = boost::iterator_range<Iterator>; 声明在NFD/daemon/table/name-tree-iterator.hpp
  Range
  partialEnumerate(const Name& prefix,
                   const EntrySubTreeSelector& entrySubTreeSelector = AnyEntrySubTree()) const;


  // return an iterator to the beginning
  const_iterator begin() const{
    return fullEnumerate().begin();
  }

  // return an iterator to the end
  const_iterator end() const{
    return Iterator();
  }

private:
  // 声明在/NFD/daemon/table/name-tree-hashtable.hpp
  // 本质上是一个vector<Node*>,每一个bucket都是一个doubly linked list
  Hashtable m_ht;

  friend class EnumerationImpl;
};

} // namespace name_tree

using name_tree::NameTree;

} // namespace nfd
```


/NFD/daemon/table/name-tree.cpp
```cpp
namespace nfd {
namespace name_tree {

NFD_LOG_INIT(NameTree);

NameTree::NameTree(size_t nBuckets)
  : m_ht(HashtableOptions(nBuckets)){
}

/*************************************lookup*********************************/
Entry& NameTree::lookup(const Name& name, size_t prefixLen){
  NFD_LOG_TRACE("lookup(" << name << ", " << prefixLen << ')');
  BOOST_ASSERT(prefixLen <= name.size());
  BOOST_ASSERT(prefixLen <= getMaxDepth());

  // 计算name各个prefix的hashvalue
  HashSequence hashes = computeHashes(name, prefixLen);
  // Node定义在NFD/daemon/table/name-tree-hashtable.hpp
  const Node* node = nullptr;

  Entry* parent = nullptr;

  // 依次遍历name的每一个prefix
  for (size_t i = 0; i <= prefixLen; ++i) {
    bool isNew = false;
    // isNew:false指示该entry是已经存在的
    // isNew::true指示该entry是新插入的
    std::tie(node, isNew) = m_ht.insert(name, i, hashes);

    // 如果name的第i个前缀是新插入的，并且parent不为空
    if (isNew && parent != nullptr) {
      // 个人理解：由于哈希映射的随机性，name的每个prefix存储的bucket位置不一样
      // 所以需要一种方式记录每个前缀的祖先在哪里
      node->entry.setParent(*parent);
    }
    parent = &node->entry;
  }
  return node->entry;
}

// 查找FIB entry
Entry& NameTree::lookup(const fib::Entry& fibEntry){
  NFD_LOG_TRACE("lookup(FIB " << fibEntry.getPrefix() << ')');
  // 获取name tree entry
  Entry* nte = this->getEntry(fibEntry);
  if (nte == nullptr) {
    // special case: Fib::s_emptyEntry is unattached
    BOOST_ASSERT(fibEntry.getPrefix().empty());
    // 调用lookup(const Name& name);
    return this->lookup(fibEntry.getPrefix());
  }

  BOOST_ASSERT(nte->getFibEntry() == &fibEntry);
  return *nte;
}

Entry& NameTree::lookup(const pit::Entry& pitEntry){
  // 获取PIT entry的name
  const Name& name = pitEntry.getName();
  NFD_LOG_TRACE("lookup(PIT " << name << ')');

  bool hasDigest = name.size() > 0 && name[-1].isImplicitSha256Digest();
  if (hasDigest && name.size() <= getMaxDepth()) {
    return this->lookup(name);
  }

  Entry* nte = this->getEntry(pitEntry);
  BOOST_ASSERT(nte != nullptr);
  BOOST_ASSERT(std::count_if(nte->getPitEntries().begin(), nte->getPitEntries().end(),
    [&pitEntry] (const shared_ptr<pit::Entry>& pitEntry1) {
      return pitEntry1.get() == &pitEntry;
    }) == 1);
  return *nte;
}

Entry& NameTree::lookup(const measurements::Entry& measurementsEntry){
  NFD_LOG_TRACE("lookup(M " << measurementsEntry.getName() << ')');
  Entry* nte = this->getEntry(measurementsEntry);
  BOOST_ASSERT(nte != nullptr);

  BOOST_ASSERT(nte->getMeasurementsEntry() == &measurementsEntry);
  return *nte;
}

Entry& NameTree::lookup(const strategy_choice::Entry& strategyChoiceEntry){
  NFD_LOG_TRACE("lookup(SC " << strategyChoiceEntry.getPrefix() << ')');
  Entry* nte = this->getEntry(strategyChoiceEntry);
  BOOST_ASSERT(nte != nullptr);

  BOOST_ASSERT(nte->getStrategyChoiceEntry() == &strategyChoiceEntry);
  return *nte;
}
/*************************************lookup*********************************/


// 返回erase的entry个数
size_t NameTree::eraseIfEmpty(Entry* entry, bool canEraseAncestors){
  BOOST_ASSERT(entry != nullptr);

  size_t nErased = 0;
  for (Entry* parent = nullptr; entry != nullptr && entry->isEmpty(); entry = parent) {
    parent = entry->getParent();

    if (parent != nullptr) {
      entry->unsetParent();
    }
    // 删除entry
    m_ht.erase(getNode(*entry));
    // 被删除的entry自增
    ++nErased;
    // 如果不允许删除祖先，直接退出；允许的话，向上访问parent
    if (!canEraseAncestors) {
      break;
    }
  }

  if (nErased == 0) {
    NFD_LOG_TRACE("not-erase " << entry->getName());
  }
  return nErased;
}

/*************************************find*************************************/
Entry* NameTree::findExactMatch(const Name& name, size_t prefixLen) const{
  prefixLen = std::min(name.size(), prefixLen);
  if (prefixLen > getMaxDepth()) {
    return nullptr;
  }

  const Node* node = m_ht.find(name, prefixLen);
  return node == nullptr ? nullptr : &node->entry;
}

Entry* NameTree::findLongestPrefixMatch(const Name& name, const EntrySelector& entrySelector) const{
  size_t depth = std::min(name.size(), getMaxDepth());
  // 计算name所有prefix的hashvalue
  HashSequence hashes = computeHashes(name, depth);

  // 最长前缀匹配，从最长的前缀开始查找
  for (ssize_t i = depth; i >= 0; --i) {
    const Node* node = m_ht.find(name, i, hashes);
    // 找到的entry符合entrySelector，满足要求返回
    if (node != nullptr && entrySelector(node->entry)) {
      return &node->entry;
    }
  }

  return nullptr;
}

Entry* NameTree::findLongestPrefixMatch(const Entry& entry1, const EntrySelector& entrySelector) const{
  Entry* entry = const_cast<Entry*>(&entry1);
  // 循环遍历entry及其parent，直到找到满足entrySelector的最长前缀entry
  while (entry != nullptr) {
    if (entrySelector(*entry)) {
      return entry;
    }
    entry = entry->getParent();
  }
  return nullptr;
}

Entry*
NameTree::findLongestPrefixMatch(const pit::Entry& pitEntry, const EntrySelector& entrySelector) const
{
  const Entry* nte = this->getEntry(pitEntry);
  BOOST_ASSERT(nte != nullptr);

  const Name& name = pitEntry.getName();
  size_t depth = std::min(name.size(), getMaxDepth());
  // 找到不小于PIT entry name size的name tree entry
  if (nte->getName().size() < pitEntry.getName().size()) {
    // PIT entry name either exceeds depth limit or ends with an implicit digest: go deeper
    // PIT entry name要么超出深度限制，要么以隐式摘要结尾
    for (size_t i = nte->getName().size() + 1; i <= depth; ++i) {
      const Entry* exact = this->findExactMatch(name, i);
      if (exact == nullptr) {
        break;
      }
      nte = exact;
    }
  }

  return this->findLongestPrefixMatch(*nte, entrySelector);
}


boost::iterator_range<NameTree::const_iterator>
NameTree::findAllMatches(const Name& name, const EntrySelector& entrySelector) const
{

  // 由于我们正在使用名称前缀哈希表，并且当前的LPM()是从全名开始实现的，并且每次减少1个组件的数量，因此我们可以在这里使用它。
  // 对于trie-like的设计，从根节点开始沿着trie走会更高效

  // 首先找到PIT表中，匹配Data name的最长名称前缀的entry
  Entry* entry = this->findLongestPrefixMatch(name, entrySelector);
  /**
  class Iterator声明在 /NFD/daemon/table/name-tree-iterator.hpp
  const_iterator end() const{
    return Iterator();
  }
  */
  // 
  return {Iterator(make_shared<PrefixMatchImpl>(*this, entrySelector), entry), end()};
}
/*************************************find*************************************/


boost::iterator_range<NameTree::const_iterator>
NameTree::fullEnumerate(const EntrySelector& entrySelector) const{

  return {Iterator(make_shared<FullEnumerationImpl>(*this, entrySelector), nullptr), end()};
}

boost::iterator_range<NameTree::const_iterator>
NameTree::partialEnumerate(const Name& prefix,
                           const EntrySubTreeSelector& entrySubTreeSelector) const{
  Entry* entry = this->findExactMatch(prefix);
  return {Iterator(make_shared<PartialEnumerationImpl>(*this, entrySubTreeSelector), entry), end()};
}

} // namespace name_tree
} // namespace nfd
```
