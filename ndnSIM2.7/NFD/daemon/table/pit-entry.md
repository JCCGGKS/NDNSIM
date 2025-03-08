/NFD/daemon/table/pit-entry.hpp
```cpp
namespace nfd {

namespace name_tree {
class Entry;
} // namespace name_tree

namespace pit {

// InRecord和OutRecord的基类都是FaceRecord

// an unordered collection of in-records
typedef std::list<InRecord> InRecordCollection;

// an unordered collection of out-records
typedef std::list<OutRecord> OutRecordCollection;

/** an Interest table entry
 *
 * 1.An Interest table entry represents either a pending Interest or a recently satisfied Interest.
 * 2.Each entry contains a collection of in-records, a collection of out-records,
    and two timers used in forwarding pipelines.
 * 3.In addition, the entry, in-records, and out-records are subclasses of StrategyInfoHost,
     which allows forwarding strategy to store arbitrary information on them.
 */
 /** an Interest table entry
 * Interest表项表示待处理的Interest或最近被满足的Interest。
 * 每个条目包含一组in-records、一组out-records和两个用于转发管道的计时器。
 * 此外，entry、In-records和out-records是StrategyInfoHost的子类,这使得转发策略可以在它们上面存储任意的信息。
 */
class Entry : public StrategyInfoHost, noncopyable
{
public:
  // 显式构造函数
  explicit Entry(const Interest& interest);

  // 获取entry中的interest
  const Interest& getInterest() const{
    return *m_interest;
  }

  // return Interest Name
  const Name& getName() const{
    return m_interest->getName();
  }

  /** return whether interest matches this entry
   *  interest : the Interest
   *  nEqualNameComps : number of initial name components guaranteed to be equal
   */
  bool canMatch(const Interest& interest, size_t nEqualNameComps = 0) const;

public: // in-record

  // return collection of in-records
  const InRecordCollection& getInRecords() const{
    return m_inRecords;
  }

  // true There is at least one in-record.This implies some downstream is waiting for Data or Nack.
  // false There is no in-record.This implies the entry is new or has been satisfied or Nacked.
  // 返回true，暗示有下游正在等待Data或者Nack
  // 返回false，暗示entry是新的或者已经被满足或者被Nacked
  bool hasInRecords() const{
    return !m_inRecords.empty();
  }

  InRecordCollection::iterator in_begin(){
    // 调用std::vector::begin()
    return m_inRecords.begin();
  }

  InRecordCollection::const_iterator in_begin() const{
    return m_inRecords.begin();
  }

  InRecordCollection::iterator in_end(){
    // 调用std::vector::end()
    return m_inRecords.end();
  }

  InRecordCollection::const_iterator in_end() const{
    return m_inRecords.end();
  }


  // get the in-record for face
  // return an iterator to the in-record, or .in_end() if it does not exist
  // 获取某个接口的in-record
  // 如果不存在返回in_end()
  InRecordCollection::iterator getInRecord(const Face& face);


  // insert or update an in-record
  // return an iterator to the new or updated in-record
  // 插入或者更新in-record
  // 返回新的或者已存在且更新的iterator
  InRecordCollection::iterator insertOrUpdateInRecord(Face& face, const Interest& interest);


  // delete the in-record for face if it exists
  // 删除face的in-record
  void deleteInRecord(const Face& face);

  // delete all in-record
  // 删除所有face的in-record
  void clearInRecords();

public: // out-record

  // return collection of in-records
  const OutRecordCollection& getOutRecords() const{
    return m_outRecords;
  }

  // true There is at least one out-record.This implies the Interest has been forwarded to some upstream,and they haven't returned Data, but may have returned Nacks.
  // false There is no out-record.This implies the Interest has not been forwarded.
  // 返回true暗示着
  // 返回false暗示着
  bool hasOutRecords() const{
    return !m_outRecords.empty();
  }

  OutRecordCollection::iterator out_begin(){
    // 调用std::vector::begin()
    return m_outRecords.begin();
  }


  OutRecordCollection::const_iterator out_begin() const{
    return m_outRecords.begin();
  }

  OutRecordCollection::iterator out_end(){
    // 调用std::vector::end()
    return m_outRecords.end();
  }

  OutRecordCollection::const_iterator out_end() const{
    return m_outRecords.end();
  }


  // get the out-record for face
  // return an iterator to the out-record, or .out_end() if it does not exist
  // 获取face的out-record
  // 如果不存在返回out_end()
  OutRecordCollection::iterator getOutRecord(const Face& face);


  // insert or update an out-record
  // return an iterator to the new or updated out-record
  // 插入或者更新out-record
  // 返回新的或者已经存在且被更新的iterator
  OutRecordCollection::iterator insertOrUpdateOutRecord(Face& face, const Interest& interest);

  // delete the out-record for  face if it exists
  // 删除face对应的out-record
  void deleteOutRecord(const Face& face);

public:

  // expiry timer
  // This timer is used in forwarding pipelines to delete the entry
  // 该定时器用于转发管道中删除表项
  scheduler::EventId expiryTimer;


  // indicate if PIT entry is satisfied
  // 指示PIT entry是否被满足
  bool isSatisfied;


  // Data freshness period
  // This field is meaningful only if isSatisfied is true
  // 只有当isSatisfied为真时，该字段才有意义
  time::milliseconds dataFreshnessPeriod;

private:
  shared_ptr<const Interest> m_interest;
  InRecordCollection m_inRecords;
  OutRecordCollection m_outRecords;

  name_tree::Entry* m_nameTreeEntry;

  // 友类声明
  friend class name_tree::Entry;
};

} // namespace pit
} // namespace nfd
```


/NFD/daemon/table/pit-entry.cpp
```cpp
namespace nfd {
namespace pit {

Entry::Entry(const Interest& interest)
  : isSatisfied(false)
  , dataFreshnessPeriod(0_ms)
  , m_interest(interest.shared_from_this())
  , m_nameTreeEntry(nullptr){
}

// 必须保证[0,nEqualNameComps)范围内的名称组件相等
bool Entry::canMatch(const Interest& interest, size_t nEqualNameComps) const{
  /**
  compare声明在/ndn-cxx/ndn-cxx/name.hpp
  int compare(size_t pos1, size_t count1,
          const Name& other, size_t pos2 = 0, size_t count2 = npos) const;
  compares [pos1, pos1+count1) components in this Name to [pos2, pos2+count2) components in other

  等效于this->getSubName(pos1, count1).compare(other.getSubName(pos2, count2));

  // indicates "until the end" in getSubName and compare
  static const size_t npos;// 声明
  const size_t Name::npos = std::numeric_limits<size_t>::max();// 定义
  */

  // 断言，所给参数interest的[0,nEqualNameComps)名称组件和当前entry中m_interest的[0,nEqualNameComps)名称组件相等
  BOOST_ASSERT(m_interest->getName().compare(0, nEqualNameComps,
                                             interest.getName(), 0, nEqualNameComps) == 0);
  // 断言成功，继续比较[nEqualNameComps,end)名称以及兴趣选择器是否想等
  return m_interest->getName().compare(nEqualNameComps, Name::npos,
                                       interest.getName(), nEqualNameComps) == 0 &&
         m_interest->getSelectors() == interest.getSelectors();
  // \todo #3162 match Link field
}

InRecordCollection::iterator Entry::getInRecord(const Face& face){
  // 调用std::find_if，详细介绍见/C++/std
  // 顺序遍历容器中的元素，返回第一个满足条件(UnaryPredicate)的元素
  return std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face] (const InRecord& inRecord) { return &inRecord.getFace() == &face; });
}

/************************************InRecord************************************************/
InRecordCollection::iterator Entry::insertOrUpdateInRecord(Face& face, const Interest& interest){

  // 断言，当前entry中的m_interest和传入参数interest相匹配
  BOOST_ASSERT(this->canMatch(interest));

  // 调用std::find_if,详解见/C++/std
  // 找到m_inRecords中和传入参数face相匹配的inRecord
  auto it = std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face] (const InRecord& inRecord) { return &inRecord.getFace() == &face; });

  // 找不到就插入
  if (it == m_inRecords.end()) {
    // 调用std::vector::emplace_front
    // 调用inRecord的构造函数，根据传入参数face构造inRecord实例并插入容器首部
    m_inRecords.emplace_front(face);
    // 获取新插入的face
    it = m_inRecords.begin();
  }
  // 找到就更新
  // 声明在/NFD/daemon/table/pit-in-record.hpp
  // 更新lastNonce, lastRenewed, expiry fields
  it->update(interest);
  return it;
}

void Entry::deleteInRecord(const Face& face){
  // 调用std::find_if
  auto it = std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face] (const InRecord& inRecord) { return &inRecord.getFace() == &face; });
  // 找到
  if (it != m_inRecords.end()) {
    // 调用std::vector::erase()
    m_inRecords.erase(it);
  }
}

void Entry::clearInRecords(){
  // 调用std::vector::clear()
  m_inRecords.clear();
}

OutRecordCollection::iterator Entry::getOutRecord(const Face& face){
  return std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face] (const OutRecord& outRecord) { return &outRecord.getFace() == &face; });
}
/************************************InRecord************************************************/

/************************************OutRecord*********************************************/
OutRecordCollection::iterator
Entry::insertOrUpdateOutRecord(Face& face, const Interest& interest){

  // 断言，当前entry中的m_interest和传入参数interest相匹配
  BOOST_ASSERT(this->canMatch(interest));

  //
  auto it = std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face] (const OutRecord& outRecord) { return &outRecord.getFace() == &face; });

  // 没有找到，插入
  if (it == m_outRecords.end()) {
    // 调用std::vector::emplace_front
    // 调用inRecord的构造函数，根据传入参数face构造inRecord实例并插入容器首部
    m_outRecords.emplace_front(face);
    it = m_outRecords.begin();
  }

  // 找到更新
  it->update(interest);
  return it;
}

void Entry::deleteOutRecord(const Face& face){

  // 调用std::find_if
  auto it = std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face] (const OutRecord& outRecord) { return &outRecord.getFace() == &face; });
  // 找到删除
  if (it != m_outRecords.end()) {
    // 调用std::vector::erase
    m_outRecords.erase(it);
  }
}
/************************************OutRecord*********************************************/

} // namespace pit
} // namespace nfd
```
