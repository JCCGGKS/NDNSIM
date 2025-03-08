# hashtable

/NFD/daemon/table/name-tree-hashtable.hpp

拉链法实现的哈希表

```cpp
namespace nfd {
namespace name_tree {

  class Entry;

  // a single HashValue
  using HashValue = size_t;

  // a sequence of hashvalue 哈希值序列
  using HashSequence = std::vector<HashValue>;

  // computes hash value of name.getPrefix(prefixLen)
  HashValue computeHash(const Name& name, size_t prefixLen = std::numeric_limits<size_t>::max());

  // computes hash values for each prefix of name.getPrefix(prefixLen)
  // return a hash sequence, where the i-th hash value equals computeHash(name, i)
  // 为name的每个名称前缀计算哈希值，返回一个哈希值序列[前缀哈希]
  HashSequence computeHashes(const Name& name, size_t prefixLen = std::numeric_limits<size_t>::max());
/**********************************Node***************************************/
  // a hashtable node[哈希表节点]
  // 一个哈希表桶中可以添加零个或多个节点。它们通过prev和next指针被组织成一个双链表。
  class Node : noncopyable
  {
  public:

    // entry.getName() == name
    // getNode(entry) == this
    Node(HashValue h, const Name& name);


    // prev == nullptr
    // next == nullptr
    ~Node();

  public:
    const HashValue hash;
    Node* prev;
    Node* next;
    // 定义在/NFD/daemon/table/name-tree-entry.hpp
    mutable Entry entry;
  };//Node本质上就是一个双链表节点
/**********************************Node***************************************/
  // 返回与entry相关联的node
  // 此函数供NameTree内部使用。
  Node* getNode(const Entry& entry);

  /** 为双链表中的每个节点调用一个给定的函数
   *  \tparam
   *  N ： either Node or const Node
   *  F ： a functor with signature void F(N*)
   *  在这个函数中删除节点是安全的
   */
  template<typename N, typename F>
  void foreachNode(N* head, const F& func){
    N* node = head; //链表头结点
    // 遍历链表
    while (node != nullptr) {
      // 为了防止func中有detach或者delete节点的操作，先记录下下一个要访问的节点
      N* next = node->next;
      // 调用函数
      func(node);
      node = next;
    }
  }

/**********************************HashtableOptions***************************/
  // 为Hashtable提供选项
  class HashtableOptions
  {
  public:

    // 显示构造函数
    // initialSize == size
    // minSize == size
    // 一维数组的长度默认是16
    explicit HashtableOptions(size_t size = 16);

  public:

    // buckets的初始数量
    size_t initialSize;

    // buckets的最小数量
    size_t minSize;

    // 如果hashtable的节点(nodes)数超过nBuckets*expandLoadFactor，将被扩容。
    float expandLoadFactor = 0.5;

    // 当hashtable被扩容时，它的新大小是nBuckets*expandFactor
    float expandFactor = 2.0;

    // 如果hashtable的节点(nodes)数少于nBuckets*shrinkLoadFactor，将被缩容
    float shrinkLoadFactor = 0.1;

    // 当hashtable缩容时，它的新大小是max(nBuckets*shrinkFactor, minSize)
    float shrinkFactor = 0.5;
  };
/**********************************HashtableOptions***************************/



/**********************************Hashtable**********************************/
/**
* a hashtable for fast exact name lookup
* The Hashtable contains a number of buckets.
* Each node is placed into a bucket determined by a hash value computed from its name.
* Hash collision is resolved through a doubly linked list in each bucket.
* The number of buckets is adjusted according to how many nodes are stored.
*/
/**
* hashtable的作用：用于快速精确名称查找
* hashtable包含一些buckets
* 每一个节点被放置在hash value对应的bucket，该hash value由它包含的name计算得出
* 哈希碰撞通过每个bucket处的双向链表解决
* buckets的数量依照hashtable存储的节点数调整
*/
class Hashtable
{
public:
  typedef HashtableOptions Options;

  explicit Hashtable(const Options& options);

  // 销毁所有的节点
  ~Hashtable();

  // 返回节点的数量
  size_t size() const{
    return m_size;
  }

  // 返回bucket的数量
  size_t getNBuckets() const{
    // 调用的是std::vector::size()
    return m_buckets.size();
  }


  // 返回hash value在hashtable中对应的索引下标
  size_t computeBucketIndex(HashValue h) const{
    return h % this->getNBuckets();
  }


  // 返回第i个bucket，前提是bucket < getNBuckets()
  const Node* getBucket(size_t bucket) const{
    BOOST_ASSERT(bucket < this->getNBuckets());
    return m_buckets[bucket]; // don't use m_bucket.at() for better performance
  }


  // 依据名字前缀找节点。前提是name.size() > prefixLen
  const Node* find(const Name& name, size_t prefixLen) const;


  // 依据名字前缀找节点。前提是name.size() > prefixLen , hashes = computeHashes(name)
  const Node* find(const Name& name, size_t prefixLen, const HashSequence& hashes) const;

  //查找或插入节点：name.getPrefix(prefixLen)
  // name.size() > prefixLen
  // hashes == computeHashes(name)
  std::pair<const Node*, bool>
  insert(const Name& name, size_t prefixLen, const HashSequence& hashes);

  // 删除node，前提是该node存在于hashtable
  void erase(Node* node);

private:

  // 把node添加到对应的bucket
  void attach(size_t bucket, Node* node);

  // 从bucket中分离node
  void detach(size_t bucket, Node* node);

  std::pair<const Node*, bool>
  findOrInsert(const Name& name, size_t prefixLen, HashValue h, bool allowInsert);

  void computeThresholds();

  void resize(size_t newNBuckets);

private:
  // Hashtable[拉链法解决哈希碰撞]
  std::vector<Node*> m_buckets;
  // 用来配置Hashtable
  Options m_options;
  // Hashtable中buckets数量
  size_t m_size;
  // 扩容阈值
  size_t m_expandThreshold;
  // 缩容阈值
  size_t m_shrinkThreshold;
};
/**********************************Hashtable**********************************/



} // namespace name_tree
} // namespace nfd
```

/NFD/daemon/table/name-tree-hashtable.cpp
```cpp
namespace nfd {
namespace name_tree {

NFD_LOG_INIT(NameTreeHashtable);

class Hash32
{
public:
  static HashValue
  compute(const void* buffer, size_t length)
  {
    return static_cast<HashValue>(CityHash32(reinterpret_cast<const char*>(buffer), length));
  }
};

class Hash64
{
public:
  static HashValue
  compute(const void* buffer, size_t length)
  {
    return static_cast<HashValue>(CityHash64(reinterpret_cast<const char*>(buffer), length));
  }
};

// 一个带有静态计算方法的类型,该方法从raw buffer中计算hash value
using HashFunc = std::conditional<(sizeof(HashValue) > 4), Hash64, Hash32>::type;

// 计算名字前缀的哈希值
HashValue computeHash(const Name& name, size_t prefixLen){

  name.wireEncode(); // ensure wire buffer exists

  HashValue h = 0;
  for (size_t i = 0, last = std::min(prefixLen, name.size()); i < last; ++i) {
    const name::Component& comp = name[i];
    // ^:是异或操作，两个比特位相同异或结果为0，不同异或结果为1
    h ^= HashFunc::compute(comp.wire(), comp.size());
  }
  return h;
}

// 计算名字各个前缀的哈希值
HashSequence computeHashes(const Name& name, size_t prefixLen){
  name.wireEncode(); // ensure wire buffer exists

  size_t last = std::min(prefixLen, name.size());
  HashSequence seq;

  // 调用std::vector::reserve,修改容器的capacity为last+1
  seq.reserve(last + 1);

  HashValue h = 0;
  seq.push_back(h);

  for (size_t i = 0; i < last; ++i) {
    const name::Component& comp = name[i];
    h ^= HashFunc::compute(comp.wire(), comp.size());
    // 这里记录的h是一个累加(异或)操作的过程，记录的是name的每个前缀的hashavalue
    seq.push_back(h);
  }
  return seq;
}

Node::Node(HashValue h, const Name& name)
  : hash(h)
  , prev(nullptr)
  , next(nullptr)
  , entry(name, this){
}

Node::~Node(){
  BOOST_ASSERT(prev == nullptr);
  BOOST_ASSERT(next == nullptr);
}

Node* getNode(const Entry& entry){
  return entry.m_node;
}

HashtableOptions::HashtableOptions(size_t size): initialSize(size), minSize(size){
}

Hashtable::Hashtable(const Options& options): m_options(options), m_size(0){
  BOOST_ASSERT(m_options.minSize > 0);
  BOOST_ASSERT(m_options.initialSize >= m_options.minSize);
  BOOST_ASSERT(m_options.expandLoadFactor > 0.0);
  BOOST_ASSERT(m_options.expandLoadFactor <= 1.0);
  BOOST_ASSERT(m_options.expandFactor > 1.0);
  BOOST_ASSERT(m_options.shrinkLoadFactor >= 0.0);
  BOOST_ASSERT(m_options.shrinkLoadFactor < 1.0);
  BOOST_ASSERT(m_options.shrinkFactor > 0.0);
  BOOST_ASSERT(m_options.shrinkFactor < 1.0);

  //调用std::vector::resize,修改容器的size
  m_buckets.resize(options.initialSize);
  // 重新计算扩容和缩容的阈值
  this->computeThresholds();
}

Hashtable::~Hashtable(){
  // 外层访问的是hashtable中的每个bucket
  for (size_t i = 0; i < m_buckets.size(); ++i) {
    // 内层遍历bucket处的doubly linked list 的每个node
    foreachNode(m_buckets[i], [] (Node* node) {
      // 前后指针置空
      node->prev = node->next = nullptr;
      // 释放节点
      delete node;
    });
  }

}
/*******************************attach：头插法***************************************/
void Hashtable::attach(size_t bucket, Node* node){
  node->prev = nullptr;
  node->next = m_buckets[bucket];

  if (node->next != nullptr) {
    BOOST_ASSERT(node->next->prev == nullptr);
    node->next->prev = node;
  }

  m_buckets[bucket] = node;
}
/*******************************attach**********************************************/


/*******************************detach**********************************************/
void Hashtable::detach(size_t bucket, Node* node){
  if (node->prev != nullptr) {
    BOOST_ASSERT(node->prev->next == node);
    node->prev->next = node->next;
  }
  else {
    // 说明node是链表中头指针指向的节点
    BOOST_ASSERT(m_buckets[bucket] == node);
    m_buckets[bucket] = node->next;
  }

  if (node->next != nullptr) {
    BOOST_ASSERT(node->next->prev == node);
    node->next->prev = node->prev;
  }
  // 置空node前后指针
  node->prev = node->next = nullptr;
}
/*******************************detach**********************************************/

// 返回的bool类型的变量，用来指示该节点是新插入的还是之前已经存在或插入失败的
// true:新插入的；false:之前已经存在或者插入失败
std::pair<const Node*, bool>
Hashtable::findOrInsert(const Name& name, size_t prefixLen, HashValue h, bool allowInsert){

  // 计算hashvalue在hashtable中对应的bucket索引下标
  size_t bucket = this->computeBucketIndex(h);

  // 首先遍历链表，检查该node是否已经存在
  for (const Node* node = m_buckets[bucket]; node != nullptr; node = node->next) {
    // 精确匹配，比较节点的hashvalue 和 具体的name
    if (node->hash == h && name.compare(0, prefixLen, node->entry.getName()) == 0) {
      NFD_LOG_TRACE("found " << name.getPrefix(prefixLen) << " hash=" << h << " bucket=" << bucket);
      return {node, false};
    }
  }

  if (!allowInsert) {
    NFD_LOG_TRACE("not-found " << name.getPrefix(prefixLen) << " hash=" << h << " bucket=" << bucket);
    return {nullptr, false};
  }

  // 头插法插入节点
  Node* node = new Node(h, name.getPrefix(prefixLen));
  this->attach(bucket, node);
  NFD_LOG_TRACE("insert " << node->entry.getName() << " hash=" << h << " bucket=" << bucket);
  // 节点个数增加
  ++m_size;

  // 扩容检查
  if (m_size > m_expandThreshold) {
    // 达到扩容条件
    this->resize(static_cast<size_t>(m_options.expandFactor * this->getNBuckets()));
  }

  return {node, true};
}

const Node* Hashtable::find(const Name& name, size_t prefixLen) const{
  HashValue h = computeHash(name, prefixLen);
  return const_cast<Hashtable*>(this)->findOrInsert(name, prefixLen, h, false).first;
}

const Node*
Hashtable::find(const Name& name, size_t prefixLen, const HashSequence& hashes) const{
  BOOST_ASSERT(hashes.at(prefixLen) == computeHash(name, prefixLen));
  return const_cast<Hashtable*>(this)->findOrInsert(name, prefixLen, hashes[prefixLen], false).first;
}

std::pair<const Node*, bool>
Hashtable::insert(const Name& name, size_t prefixLen, const HashSequence& hashes){
  BOOST_ASSERT(hashes.at(prefixLen) == computeHash(name, prefixLen));
  return this->findOrInsert(name, prefixLen, hashes[prefixLen], true);
}

/*******************************erase**********************************************/
void Hashtable::erase(Node* node){
  BOOST_ASSERT(node != nullptr);
  BOOST_ASSERT(node->entry.getParent() == nullptr);

  size_t bucket = this->computeBucketIndex(node->hash);
  NFD_LOG_TRACE("erase " << node->entry.getName() << " hash=" << node->hash << " bucket=" << bucket);

  this->detach(bucket, node);
  delete node;
  --m_size;

  // 判断是否达到缩容条件，
  if (m_size < m_shrinkThreshold) {
    // 达到缩容条件，缩容之后hashtable的大小
    size_t newNBuckets = std::max(m_options.minSize,
      static_cast<size_t>(m_options.shrinkFactor * this->getNBuckets()));
    // 直接修改hashtable的长度[个人感觉，这个做法有些暴力]
    this->resize(newNBuckets);
  }
}
/*******************************erase**********************************************/

/*******************************computeThresholds*********************************/
//
void Hashtable::computeThresholds(){
  // 计算扩容的阈值 = 扩容负载因子 * number of bucket in hashtable
  m_expandThreshold = static_cast<size_t>(m_options.expandLoadFactor * this->getNBuckets());
  // 计算缩容的阈值 = 缩容负载因子 * number of bucket in hashtable
  m_shrinkThreshold = static_cast<size_t>(m_options.shrinkLoadFactor * this->getNBuckets());
  NFD_LOG_TRACE("thresholds expand=" << m_expandThreshold << " shrink=" << m_shrinkThreshold);
}
/*******************************computeThresholds*********************************/

void Hashtable::resize(size_t newNBuckets){

  if (this->getNBuckets() == newNBuckets) {
    return;
  }
  NFD_LOG_DEBUG("resize from=" << this->getNBuckets() << " to=" << newNBuckets);

  std::vector<Node*> oldBuckets;
  // 调用std::vector::swap,交换两个容器中存储的内容。
  // 这一步的操作是将旧的hashtable先放置到一个临时的vector
  oldBuckets.swap(m_buckets);
  // 修改容器的size为new_size = newNBuckets
  m_buckets.resize(newNBuckets);

  // 重新计算hashvalue，将临时表中的节点依次迁移到bucket中
  for (Node* head : oldBuckets) {
    foreachNode(head, [this] (Node* node) {
      size_t bucket = this->computeBucketIndex(node->hash);
      this->attach(bucket, node);
    });
  }
  // 重新计算扩容和缩容的阈值
  this->computeThresholds();
}

} // namespace name_tree
} // namespace nfd
```
