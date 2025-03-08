/NFD/daemon/table/cs-entry-impl.hpp
```cpp
namespace nfd {
namespace cs {

/** an Entry in ContentStore implementation
 *
 *  An Entry is either a stored Entry which contains a Data packet and related attributes,
 *  or a query Entry which contains a Name that is LessComparable to other stored/query Entry
 *  and is used to lookup a container of entries.
 *
 *  \note This type is internal to this specific ContentStore implementation.
 */
 /**  
 在内容存储（ContentStore）实现中的一个条目（Entry）：
 条目要么是一个存储的条目，它包含一个数据包（Data packet）和相关的属性；
 要么是一个查询条目，它包含一个Name，这个Name与其他存储/查询条目的Name用LessComparable相比较，用于查找条目容器。

 这个类型是这个特定ContentStore实现的内部类型。
 */
class EntryImpl : public Entry
{
public:
  /** construct Entry for query
   *  Name is implicitly convertible to Entry, so that Name can be passed to lookup functions on a container of Entry
   *  Name隐式地转换为Entry，因此可以将Name传递给使用Entry容器的查找函数
   */
  // 可以隐式转换的构造函数
  EntryImpl(const Name& name);

  // construct Entry for storage
  EntryImpl(shared_ptr<const Data> data, bool isUnsolicited);

  void unsetUnsolicited();

  bool operator<(const EntryImpl& other) const;

private:
  bool isQuery() const;

private:
  Name m_queryName;
};

} // namespace cs
} // namespace nfd
```


/NFD/daemon/table/cs-entry-impl.cpp
```cpp
namespace nfd {
namespace cs {

// 隐式构造函数
EntryImpl::EntryImpl(const Name& name)
  : m_queryName(name){
  // 断言该实例没有data
  BOOST_ASSERT(this->isQuery());
}

EntryImpl::EntryImpl(shared_ptr<const Data> data, bool isUnsolicited){
  this->setData(data, isUnsolicited);
  BOOST_ASSERT(!this->isQuery());
}

bool EntryImpl::isQuery() const{
  return !this->hasData();
}

// 设置Unsolicited属性为false
void EntryImpl::unsetUnsolicited(){
  BOOST_ASSERT(!this->isQuery());
  this->setData(this->getData(), false);
}

/*******************************************************************************/
// name 与 data比较
int compareQueryWithData(const Name& queryName, const Data& data){
  // 判断给定的name是否包含隐式摘要
  bool queryIsFullName = !queryName.empty() && queryName[-1].isImplicitSha256Digest();

  // 比较不包含隐式摘要的name
  int cmp = queryIsFullName ?
            queryName.compare(0, queryName.size() - 1, data.getName()) :
            queryName.compare(data.getName());

  if (cmp != 0) { // Name without digest differs
    return cmp;
  }

  if (queryIsFullName) { // Name without digest equals, compare digest
    return queryName[-1].compare(data.getFullName()[-1]);
  }
  else { // queryName is a proper prefix of Data fullName
    return -1;
  }
}

// data 与 data比较
int compareDataWithData(const Data& lhs, const Data& rhs){
  int cmp = lhs.getName().compare(rhs.getName());
  if (cmp != 0) {
    return cmp;
  }
  // name相等继续比较name最后的隐式摘要
  return lhs.getFullName()[-1].compare(rhs.getFullName()[-1]);
}

bool EntryImpl::operator<(const EntryImpl& other) const{
  if (this->isQuery()) {
    if (other.isQuery()) {
      // this 和 other都没有data，直接比较name
      return m_queryName < other.m_queryName;
    }
    else {
      return compareQueryWithData(m_queryName, other.getData()) < 0;
    }
  }
  else {
    if (other.isQuery()) {
      return compareQueryWithData(other.m_queryName, this->getData()) > 0;
    }
    else {
      // this 和 other 都有data，直接比较data
      return compareDataWithData(this->getData(), other.getData()) < 0;
    }
  }
}
/***********************************************************************/
} // namespace cs
} // namespace nfd
```
