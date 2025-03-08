/NFD/daemon/table/fib-entry.hpp
```cpp
namespace nfd {

namespace name_tree {
class Entry; // name tree entry
} // namespace name_tree

namespace fib {

/** \class nfd::fib::NextHopList
 *  \brief Represents a collection of nexthops.
 *
 *  This type has the following member functions:
 *  - `iterator<NextHop> begin()`
 *  - `iterator<NextHop> end()`
 *  - `size_t size()`
 */
// NextHop声明在/NFD/daemon/table/fib-nexthop.hpp
using NextHopList = std::vector<NextHop>;


// represents a FIB entry
class Entry : noncopyable
{
public:
  // 显示构造函数
  explicit Entry(const Name& prefix);

  const Name& getPrefix() const{
    return m_prefix;
  }

  const NextHopList& getNextHops() const{
    return m_nextHops;
  }

  // return whether this Entry has any NextHop record
  bool hasNextHops() const{
    return !m_nextHops.empty();
  }

  // return whether there is a NextHop record for face with the given endpointId
  bool hasNextHop(const Face& face, uint64_t endpointId) const;

  // adds a NextHop record
  // If a NextHop record for face and endpointId already exists,its cost is updated.
  void addOrUpdateNextHop(Face& face, uint64_t endpointId, uint64_t cost);

  // removes the NextHop record for face with the given endpointId
  void removeNextHop(const Face& face, uint64_t endpointId);

  // removes all NextHop records on face for any endpointId
  void removeNextHopByFace(const Face& face);

private:

  // This method is non-const because mutable iterators are needed by callers.
  NextHopList::iterator findNextHop(const Face& face, uint64_t endpointId);

  // sorts the nexthop list by cost
  void sortNextHops();

private:
  Name m_prefix;
  NextHopList m_nextHops;

  name_tree::Entry* m_nameTreeEntry;

  friend class name_tree::Entry;
};

} // namespace fib
} // namespace nfd
```

/NFD/daemon/table/fib-entry.cpp
```cpp
namespace nfd {
namespace fib {

Entry::Entry(const Name& prefix)
  : m_prefix(prefix)
  , m_nameTreeEntry(nullptr){
}

NextHopList::iterator
Entry::findNextHop(const Face& face, uint64_t endpointId){
  // 顺序查找，时间复杂度为O(n);
  // 返回给定范围区间中，第一个使得第三个参数：lambda函数为true的元素，如果没有的话返回end()
  // 详解见/C++/std/find_if
  return std::find_if(m_nextHops.begin(), m_nextHops.end(),
                      [&face, endpointId] (const NextHop& nexthop) {
                        return &nexthop.getFace() == &face && nexthop.getEndpointId() == endpointId;
                      });
}

bool Entry::hasNextHop(const Face& face, uint64_t endpointId) const{
  // this在const成员函数中是一个指针常量(指向常量的指针，const Entry*，底层常量)
  // 在const成员函数中调用一个非const成员函数，或者需要修改对象的状态，你必须去掉this指针的const属性
  // 所以需要先用const_cast强制去掉this的底层const属性
  // 有关const_cast的详解可见/C++/type_cast
  return const_cast<Entry*>(this)->findNextHop(face, endpointId) != m_nextHops.end();
}

void Entry::addOrUpdateNextHop(Face& face, uint64_t endpointId, uint64_t cost){
  auto it = this->findNextHop(face, endpointId);
  // 没找到就插入
  if (it == m_nextHops.end()) {
    m_nextHops.emplace_back(face, endpointId);
    it = std::prev(m_nextHops.end());
  }
  // 找到就更新cost，没找到就设置cost
  it->setCost(cost);
  // 根据cost排序
  // 如果每次add或者update就要重新排序的话，时间复杂度其实挺高的
  this->sortNextHops();
}

void Entry::removeNextHop(const Face& face, uint64_t endpointId){
  auto it = this->findNextHop(face, endpointId);
  if (it != m_nextHops.end()) {
    m_nextHops.erase(it);
  }
}


void Entry::removeNextHopByFace(const Face& face){
  //std::remove_if是通过将不满足条件即不会被删除的元素移动到容器的最前面并返回最后一个未被删除元素的下一个位置
  //这个位置直到容器end的元素是什么不重要，因此还需要用erase将这些元素从容器中删除
  //对于std::remove_if的详解可以参考/C++/std/remove_if
  auto it = std::remove_if(m_nextHops.begin(), m_nextHops.end(),
                           [&face] (const NextHop& nexthop) {
                             return &nexthop.getFace() == &face;
                           });

  // 调用std::vector::erase(const_iterator first, const_iterator last)
  m_nextHops.erase(it, m_nextHops.end());
}

// sort by cost
void Entry::sortNextHops(){
  // 按照cost从小到大排序
  std::sort(m_nextHops.begin(), m_nextHops.end(),
            [] (const NextHop& a, const NextHop& b) { return a.getCost() < b.getCost(); });
}

} // namespace fib
} // namespace nfd
```
