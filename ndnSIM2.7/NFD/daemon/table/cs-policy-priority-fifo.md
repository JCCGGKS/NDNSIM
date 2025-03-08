/NFD/daemon/table/cs-policy-priority-fifo.hpp

```cpp
namespace nfd {
namespace cs {
namespace priority_fifo {

typedef std::list<iterator> Queue;
typedef Queue::iterator QueueIt;

// 无限定作用域的枚举类型
enum QueueType {
  QUEUE_UNSOLICITED,
  QUEUE_STALE,
  QUEUE_FIFO,
  QUEUE_MAX
};

struct EntryInfo
{
  QueueType queueType;
  QueueIt queueIt;
  scheduler::EventId moveStaleEventId;
};

struct EntryItComparator
{
  bool
  operator()(const iterator& a, const iterator& b) const
  {
    return *a < *b;
  }
};

typedef std::map<iterator, EntryInfo*, EntryItComparator> EntryInfoMapFifo;

/** Priority FIFO replacement policy
 *
 *  This policy maintains a set of cleanup queues to decide the eviction order of CS entries.
 *  The cleanup queues are three doubly linked lists that store Table iterators.
 *  The three queues keep track of unsolicited, stale, and fresh Data packet, respectively.
 *  Table iterator is placed into, removed from, and moved between suitable queues
 *  whenever an Entry is added, removed, or has other attribute changes.
 *  The Table iterator of an Entry should be in exactly one queue at any moment.
 *  Within each queue, the iterators are kept in first-in-first-out order.
 *  Eviction procedure exhausts the first queue before moving onto the next queue,
 *  in the order of unsolicited, stale, and fresh queue.
 */
 /**
 * 该策略维护一组清理队列，以决定CS项的清除顺序。
 * 清理队列是三个存储Table迭代器的双链表。
 * 这三个队列分别跟踪未请求的、过期的和新鲜的数据包。
 * 每当添加、删除条目或更改其他属性时，将表迭代器放入、移除并在合适的队列之间移动。
 * Entry的Table迭代器在任何时候都应该恰好在一个队列中。
 * 在每个队列中，迭代器保持先进先出的顺序。
 * 驱逐过程在移动到下一个队列之前耗尽第一个队列，顺序为未请求队列、过期队列和新队列。
 */
class PriorityFifoPolicy : public Policy
{
public:
  PriorityFifoPolicy();

  virtual  ~PriorityFifoPolicy();

public:
  static const std::string POLICY_NAME;

private:
  void doAfterInsert(iterator i) override;

  void doAfterRefresh(iterator i) override;

  void doBeforeErase(iterator i) override;

  void doBeforeUse(iterator i) override;

  void evictEntries() override;

private:

  // evicts one entry 前提是CS is not empty
  void evictOne();

  // attaches the entry to an appropriate queue
  // 前提是the entry is not in any queue
  void attachQueue(iterator i);

  // detaches the entry from its current queue
  // 执行操作之后： the entry is not in any queue
  void detachQueue(iterator i);

  // moves an entry from FIFO queue to STALE queue
  void moveToStaleQueue(iterator i);

private:
  Queue m_queues[QUEUE_MAX];
  EntryInfoMapFifo m_entryInfoMap;
};

} // namespace priority_fifo

using priority_fifo::PriorityFifoPolicy;

} // namespace cs
} // namespace nfd
```


/NFD/daemon/table/cs-policy-priority-fifo.cpp
```cpp
namespace nfd {
namespace cs {
namespace priority_fifo {

// 静态变量类外定义
const std::string PriorityFifoPolicy::POLICY_NAME = "priority_fifo";

NFD_REGISTER_CS_POLICY(PriorityFifoPolicy);

PriorityFifoPolicy::PriorityFifoPolicy()
  : Policy(POLICY_NAME){
}

// 析构函数，delete由new创造出来的变量，释放内存
PriorityFifoPolicy::~PriorityFifoPolicy()
{
  for (auto entryInfoMapPair : m_entryInfoMap) {
    delete entryInfoMapPair.second;
  }
}

void PriorityFifoPolicy::doAfterInsert(iterator i){
  // 加入queue
  this->attachQueue(i);
  // 必要时，执行驱逐操作
  this->evictEntries();
}

void PriorityFifoPolicy::doAfterRefresh(iterator i){
  // 从原位置删除
  this->detachQueue(i);
  // 加入到新的位置
  this->attachQueue(i);
}

void PriorityFifoPolicy::doBeforeErase(iterator i){
  this->detachQueue(i);
}

void PriorityFifoPolicy::doBeforeUse(iterator i){
  // 用之前，先断言iterator i存在
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());
}

void PriorityFifoPolicy::evictEntries(){

  BOOST_ASSERT(this->getCs() != nullptr);

  while (this->getCs()->size() > this->getLimit()) {
    this->evictOne();
  }
}

// 驱逐顺序是unsolicited_queue 、stale_queue、fifo_queue
void PriorityFifoPolicy::evictOne(){
  BOOST_ASSERT(!m_queues[QUEUE_UNSOLICITED].empty() ||
               !m_queues[QUEUE_STALE].empty() ||
               !m_queues[QUEUE_FIFO].empty());

  iterator i;
  if (!m_queues[QUEUE_UNSOLICITED].empty()) {
    i = m_queues[QUEUE_UNSOLICITED].front();
  }
  else if (!m_queues[QUEUE_STALE].empty()) {
    i = m_queues[QUEUE_STALE].front();
  }
  else if (!m_queues[QUEUE_FIFO].empty()) {
    i = m_queues[QUEUE_FIFO].front();
  }

  this->detachQueue(i);
  this->emitSignal(beforeEvict, i);
}

/*******************************************************************/
void PriorityFifoPolicy::attachQueue(iterator i){
  // 前提是不存在
  BOOST_ASSERT(m_entryInfoMap.find(i) == m_entryInfoMap.end());

  // new一个EntryInfo,之后用delete释放
  EntryInfo* entryInfo = new EntryInfo();
  if (i->isUnsolicited()) {
    entryInfo->queueType = QUEUE_UNSOLICITED;
  }
  else if (i->isStale()) {
    entryInfo->queueType = QUEUE_STALE;
  }
  else {
    entryInfo->queueType = QUEUE_FIFO;
    entryInfo->moveStaleEventId = scheduler::schedule(i->getData().getFreshnessPeriod(),
                                                      [=] { moveToStaleQueue(i); });
  }

  Queue& queue = m_queues[entryInfo->queueType];
  // 调用的是std::list::insert
  entryInfo->queueIt = queue.insert(queue.end(), i);
  // 在map中建立映射
  m_entryInfoMap[i] = entryInfo;
}

void PriorityFifoPolicy::detachQueue(iterator i){

  // 前提是存在
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];
  if (entryInfo->queueType == QUEUE_FIFO) {
    scheduler::cancel(entryInfo->moveStaleEventId);
  }

  m_queues[entryInfo->queueType].erase(entryInfo->queueIt);
  m_entryInfoMap.erase(i);
  delete entryInfo;
}

void PriorityFifoPolicy::moveToStaleQueue(iterator i){

  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];
  BOOST_ASSERT(entryInfo->queueType == QUEUE_FIFO);

  m_queues[QUEUE_FIFO].erase(entryInfo->queueIt);

  entryInfo->queueType = QUEUE_STALE;
  Queue& queue = m_queues[QUEUE_STALE];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;
}
/*****************************************************************/

} // namespace priority_fifo
} // namespace cs
} // namespace nfd
```
