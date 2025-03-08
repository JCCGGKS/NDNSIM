/NFD/daemon/table/name-tree-entry.hpp
```cpp
namespace nfd {
namespace name_tree {

class Node;

// an entry in the name tree
class Entry : noncopyable
{
public:
  Entry(const Name& prefix, Node* node);

  const Name& getName() const{
    return m_name;
  }


  // 如果entry是root entry(getName()==Name())，返回nullptr
  Entry* getParent() const{
    return m_parent;
  }

  /** 设置entry的parent
   *  \param entry ： entry of getName().getPrefix(-1)
   *  前提是： getParent() == nullptr
   *  \post getParent() == &entry
   *  \post entry.getChildren() contains this
   */
  void setParent(Entry& entry);

  /** 不设置entry的parent
   *  \post getParent() == nullptr
   *  \post parent.getChildren() does not contain this
   */
  void unsetParent();


  // true ： this entry has at least one child
  // false ： this entry has no children
  bool hasChildren() const{
    // 调用std::vector::empty
    return !this->getChildren().empty();
  }

  // return children of this entry
  const std::vector<Entry*>& getChildren() const{
    return m_children;
  }

  // true : this entry has no children and no table entries
  // false : this entry has child or attached table entry
  bool isEmpty() const{
    return !this->hasChildren() && !this->hasTableEntries();
  }

public: // attached table entries

  // true : at least one table entries is attached
  // false :  no table entry is attached
  bool hasTableEntries() const;

  fib::Entry* getFibEntry() const{
    // 调用std::unique_ptr::get
    return m_fibEntry.get();
  }

  void setFibEntry(unique_ptr<fib::Entry> fibEntry);

  bool hasPitEntries() const{
    // 调用的是std::vector::empty
    return !this->getPitEntries().empty();
  }

  const std::vector<shared_ptr<pit::Entry>>&
  getPitEntries() const{
    return m_pitEntries;
  }

  void insertPitEntry(shared_ptr<pit::Entry> pitEntry);

  void erasePitEntry(pit::Entry* pitEntry);

  measurements::Entry* getMeasurementsEntry() const{
    return m_measurementsEntry.get();
  }

  void setMeasurementsEntry(unique_ptr<measurements::Entry> measurementsEntry);

  strategy_choice::Entry* getStrategyChoiceEntry() const{
    return m_strategyChoiceEntry.get();
  }

  void setStrategyChoiceEntry(unique_ptr<strategy_choice::Entry> strategyChoiceEntry);

  // 此函数供NameTree内部使用。其他组件应该使用NameTree::getEntry(tableEntry)。
  template<typename ENTRY>
  static Entry* get(const ENTRY& tableEntry){
    return tableEntry.m_nameTreeEntry;
  }

private:
  Name m_name;
  Node* m_node;

  Entry* m_parent;
  std::vector<Entry*> m_children;

  unique_ptr<fib::Entry> m_fibEntry;
  std::vector<shared_ptr<pit::Entry>> m_pitEntries;
  unique_ptr<measurements::Entry> m_measurementsEntry;
  unique_ptr<strategy_choice::Entry> m_strategyChoiceEntry;

  // 友元函数声明
  friend Node* getNode(const Entry& entry);
};


/*********************************************************************************/

// 从name tree entry中获取table entry的仿函数
// 附加到名称树表项的单表项的表项类型，如fib:: ENTRY
template<typename ENTRY>
class GetTableEntry
{
public:
  // a function pointer to the getter on Entry class that returns ENTRY
  // Entry::* 这种语法表示一个成员函数指针。这种指针指向类Entry的一个成员函数。
  using Getter = ENTRY* (Entry::*)() const;

  // 需要默认实参来确保FIB和StrategyChoice迭代器是默认可构造的。
  explicit GetTableEntry(Getter getter = nullptr) : m_getter(getter){
  }

  const ENTRY& operator()(const Entry& nte) const{
    // 对象访问成员函数使用".*"
    return *(nte.*m_getter)();
  }

private:
  Getter m_getter;
};
/*************************************************************************************/

} // namespace name_tree
} // namespace nfd
```

/NFD/daemon/table/name-tree-entry.cpp
```cpp
namespace nfd {
namespace name_tree {

Entry::Entry(const Name& name, Node* node)
  : m_name(name)
  , m_node(node)
  , m_parent(nullptr){

  BOOST_ASSERT(node != nullptr);
  BOOST_ASSERT(name.size() <= NameTree::getMaxDepth());
}

void Entry::setParent(Entry& entry){
  BOOST_ASSERT(this->getParent() == nullptr);
  BOOST_ASSERT(!this->getName().empty());

  // getPrefix定义在ndn-cxx/name.hpp
  BOOST_ASSERT(entry.getName() == this->getName().getPrefix(-1));

  // 设置parent
  m_parent = &entry;

  // 将this加入到m_parent的m_children
  m_parent->m_children.push_back(this);
}

void Entry::unsetParent(){
  BOOST_ASSERT(this->getParent() != nullptr);

  //在m_children:vector<Entry*>中找到this
  auto i = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
  BOOST_ASSERT(i != m_parent->m_children.end());
  //在_children中删除this
  m_parent->m_children.erase(i);
  // 将m_parent置为空
  m_parent = nullptr;
}

bool Entry::hasTableEntries() const{
  return m_fibEntry != nullptr ||
         !m_pitEntries.empty() ||
         m_measurementsEntry != nullptr ||
         m_strategyChoiceEntry != nullptr;
}

void Entry::setFibEntry(unique_ptr<fib::Entry> fibEntry){
  BOOST_ASSERT(fibEntry == nullptr || fibEntry->m_nameTreeEntry == nullptr);

  if (m_fibEntry != nullptr) {
    m_fibEntry->m_nameTreeEntry = nullptr;
  }
  // 移动赋值函数，转移所有权
  m_fibEntry = std::move(fibEntry);

  if (m_fibEntry != nullptr) {
    m_fibEntry->m_nameTreeEntry = this;
  }
}

void Entry::insertPitEntry(shared_ptr<pit::Entry> pitEntry){
  BOOST_ASSERT(pitEntry != nullptr);
  BOOST_ASSERT(pitEntry->m_nameTreeEntry == nullptr);

  m_pitEntries.push_back(pitEntry);
  pitEntry->m_nameTreeEntry = this;
}

void Entry::erasePitEntry(pit::Entry* pitEntry){
  BOOST_ASSERT(pitEntry != nullptr);
  BOOST_ASSERT(pitEntry->m_nameTreeEntry == this);

  // 在m_pitEntries中找到pitEntry
  auto it = std::find_if(m_pitEntries.begin(), m_pitEntries.end(),
    [pitEntry] (const shared_ptr<pit::Entry>& pitEntry2) { return pitEntry2.get() == pitEntry; });
  BOOST_ASSERT(it != m_pitEntries.end());

  pitEntry->m_nameTreeEntry = nullptr; // must be done before pitEntry is deallocated
  *it = m_pitEntries.back(); // may deallocate pitEntry
  m_pitEntries.pop_back();
}

void Entry::setMeasurementsEntry(unique_ptr<measurements::Entry> measurementsEntry){
  BOOST_ASSERT(measurementsEntry == nullptr || measurementsEntry->m_nameTreeEntry == nullptr);

  if (m_measurementsEntry != nullptr) {
    m_measurementsEntry->m_nameTreeEntry = nullptr;
  }
  m_measurementsEntry = std::move(measurementsEntry);

  if (m_measurementsEntry != nullptr) {
    m_measurementsEntry->m_nameTreeEntry = this;
  }
}

void Entry::setStrategyChoiceEntry(unique_ptr<strategy_choice::Entry> strategyChoiceEntry){
  BOOST_ASSERT(strategyChoiceEntry == nullptr || strategyChoiceEntry->m_nameTreeEntry == nullptr);

  if (m_strategyChoiceEntry != nullptr) {
    m_strategyChoiceEntry->m_nameTreeEntry = nullptr;
  }
  m_strategyChoiceEntry = std::move(strategyChoiceEntry);

  if (m_strategyChoiceEntry != nullptr) {
    m_strategyChoiceEntry->m_nameTreeEntry = this;
  }
}

} // namespace name_tree
} // namespace nfd
```
