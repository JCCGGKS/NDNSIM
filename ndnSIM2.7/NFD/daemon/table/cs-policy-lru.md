/NFD/daemon/table/cs-policy-lru.hpp
```cpp
namespace nfd {
namespace cs {
namespace lru {

// Cs entry 比较函数
struct EntryItComparator
{
  /**
  * 声明在cs-internal.hpp
  * typedef std::set<EntryImpl> Table;
  * typedef Table::const_iterator iterator;
  */
  // 运算符重载
  bool operator()(const iterator& a, const iterator& b) const{
    // 调用的是bool EntryImpl::operator<(const EntryImpl& other) const
    // 比较的是Name
    return *a < *b;
  }
};

/**
* `boost::multi_index_container` 被用来创建一个名为 `Queue` 的容器，这个容器具有两种索引方式：一个是基于序列的索引（`boost::multi_index::sequenced<>`），另一个是基于唯一身份的索引（`boost::multi_index::ordered_unique`），其中元素的身份由 `boost::multi_index::identity<iterator>` 提供，并且使用 `EntryItComparator` 作为比较器。

* 分解这个声明：

* `iterator`：是 `multi_index_container` 将要存储的元素类型。

* `boost::multi_index::indexed_by`：这是一个模板参数，它允许 `multi_index_container` 通过多种方式被索引。

* `boost::multi_index::sequenced<>`：为容器中的元素提供了一个基于顺序的索引。元素将按照它们被插入的顺序进行排序

* `boost::multi_index::ordered_unique`：为容器中的元素提供了一个基于唯一性的有序索引。意味着每个元素在索引中都是唯一的。

* `boost::multi_index::identity<iterator>`：这是一个函数对象，它返回传递给它的迭代器所指向的值。在这里，它被用作 `ordered_unique` 索引的键提取器。

* `EntryItComparator`：这是一个比较器，用于在 `ordered_unique` 索引中比较元素。这个比较器必须提供一个严格弱顺序，通常是通过重载 `operator()` 实现的。

* `Queue`**：这是定义的 `multi_index_container` 类型的新名字。

* 这个 `Queue` 容器可以同时按照元素的插入顺序和元素的唯一键来访问元素。这在需要多种访问方式的场景中非常有用，例如，你可能需要按照元素的到达顺序处理元素，同时也需要能够快速检查某个元素是否已经存在于容器中。

* 在 boost::multi_index_container 中，容器中的元素的默认排序方式取决于你使用的迭代器类型。如果你使用了 boost::multi_index::sequenced<> 作为第一个索引，那么容器将按照元素插入的顺序来维护元素的排序。
*/
typedef boost::multi_index_container<
    iterator,
    boost::multi_index::indexed_by<
      boost::multi_index::sequenced<>,
      boost::multi_index::ordered_unique<
        boost::multi_index::identity<iterator>, EntryItComparator
      >
    >
  > Queue;

/** LRU cs replacement policy
 *
 * The least recently used entries get removed first.
 * Everytime when any entry is used or refreshed, Policy should witness the usage of it

 * 最近最少使用的条目首先被删除。
 * 每次使用或刷新任何条目时，Policy都应该记录它的使用情况
 */
class LruPolicy : public Policy
{
public:
  LruPolicy();

public:
  static const std::string POLICY_NAME;

private:
  virtual void doAfterInsert(iterator i) override;


  virtual void doAfterRefresh(iterator i) override;


  virtual void doBeforeErase(iterator i) override;


  virtual void doBeforeUse(iterator i) override;

  virtual void evictEntries() override;

private:

  // moves an entry to the end of queue
  void insertToQueue(iterator i, bool isNewEntry);

private:
  Queue m_queue;
};

} // namespace lru

using lru::LruPolicy;

} // namespace cs
} // namespace nfd
```

/NFD/daemon/table/cs-policy-lru.cpp

```cpp
namespace nfd {
namespace cs {
namespace lru {

// 静态成员变量类外定义
const std::string LruPolicy::POLICY_NAME = "lru";

NFD_REGISTER_CS_POLICY(LruPolicy);

LruPolicy::LruPolicy()
  : Policy(POLICY_NAME){
}

void LruPolicy::doAfterInsert(iterator i){
  // 先插入队列末端
  this->insertToQueue(i, true);
  // 有必要的话驱逐entry
  this->evictEntries();
}

void LruPolicy::doAfterRefresh(iterator i){
  this->insertToQueue(i, false);
}

void LruPolicy::doBeforeErase(iterator i){
  // get<0>() 将会返回一个类型，它允许你按照 sequenced 索引来访问元素，这是按照元素插入顺序的索引。
  // get<1>() 将会返回一个类型，它允许你按照 ordered_unique 索引来访问元素，这是按照元素的唯一键来访问的索引。
  m_queue.get<1>().erase(i);
}

void LruPolicy::doBeforeUse(iterator i){
  this->insertToQueue(i, false);
}

void LruPolicy::evictEntries(){
  BOOST_ASSERT(this->getCs() != nullptr);
  // 只有达到内存限制才会执行内存淘汰操作，驱逐entry
  while (this->getCs()->size() > this->getLimit()) {
    BOOST_ASSERT(!m_queue.empty());
    iterator i = m_queue.front();
    m_queue.pop_front();
    this->emitSignal(beforeEvict, i);
  }
}

void LruPolicy::insertToQueue(iterator i, bool isNewEntry){
  Queue::iterator it;
  bool isNew = false;
  // push_back only if iterator i does not exist
  // 只有当iterator不存在时执行push_back操作
  std::tie(it, isNew) = m_queue.push_back(i);

  // isNew为true表示iterator i不存在
  // isNew为false表示iterator i已经存在
  BOOST_ASSERT(isNew == isNewEntry);
  // 存在，更换现有位置
  if (!isNewEntry) {
    // 重新分配位置，将iterator it移动到队列末端
    m_queue.relocate(m_queue.end(), it);
  }
}

} // namespace lru
} // namespace cs
} // namespace nfd

```
