NFD/daemon/table/cs-entry.hpp
```cpp
namespace nfd {
namespace cs {

/** \brief represents a base class for CS entry
 */
class Entry
{
public: // exposed through ContentStore enumeration

  // return the stored Data
  const Data& getData() const{
    BOOST_ASSERT(this->hasData());
    return *m_data;
  }

  // return Name of the stored Data
  const Name& getName() const{
    BOOST_ASSERT(this->hasData());
    return m_data->getName();
  }

  // return full name (including implicit digest) of the stored Data
  const Name& getFullName() const{
    BOOST_ASSERT(this->hasData());
    return m_data->getFullName();
  }

  // return whether the stored Data is unsolicited
  // [unsolicited:未经请求的]
  bool isUnsolicited() const{
    BOOST_ASSERT(this->hasData());
    return m_isUnsolicited;
  }

  //返回Data过时的绝对时间
  const time::steady_clock::TimePoint&  getStaleTime() const{
    BOOST_ASSERT(this->hasData());
    return m_staleTime;
  }

  // 检查存储的Data Packet是否过期
  bool isStale() const;

  // 判断存储的Data Packet能否满足Interest Packet
  bool canSatisfy(const Interest& interest) const;

public: // used by generic ContentStore implementation

  bool hasData() const{
    return m_data != nullptr;
  }

  // replaces the stored Data
  void setData(shared_ptr<const Data> data, bool isUnsolicited);

  // replaces the stored Data
  void setData(const Data& data, bool isUnsolicited){
    this->setData(data.shared_from_this(), isUnsolicited);
  }

  // 相对于当前时间刷新过期时间
  void updateStaleTime();


  // clear the entry
  void reset();

private:
  shared_ptr<const Data> m_data;
  bool m_isUnsolicited;
  time::steady_clock::TimePoint m_staleTime;
};

} // namespace cs
} // namespace nfd
```
NFD/daemon/table/cs-entry.cpp
```cpp
namespace nfd {
namespace cs {

void Entry::setData(shared_ptr<const Data> data, bool isUnsolicited)
{
  m_data = data;
  m_isUnsolicited = isUnsolicited;

  // 更新Data的陈旧时间
  updateStaleTime();
}

bool Entry::isStale() const{
  BOOST_ASSERT(this->hasData());
  // 如果当前的时间大于Data陈旧的绝对时间，说明该Data已经过期
  return m_staleTime < time::steady_clock::now();
}

void Entry::updateStaleTime(){
  BOOST_ASSERT(this->hasData());
  // steady_clock是一种稳定性的时钟不会受系统时钟的影响，会一直单调递增
  m_staleTime = time::steady_clock::now() + time::milliseconds(m_data->getFreshnessPeriod());
}

bool Entry::canSatisfy(const Interest& interest) const{
  // 断言m_data!=nullptr
  BOOST_ASSERT(this->hasData());

  // 当前entry中存储的Data不能满足interest的要求，返回false
  if (!interest.matchesData(*m_data)) {
    return false;
  }

  // 如果兴趣包要求必须新鲜并且此刻数据包已经过期，返回false
  if (interest.getMustBeFresh() == static_cast<int>(true) && this->isStale()) {
    return false;
  }

  return true;
}

void Entry::reset(){
  // 调用的是std::shared_ptr::reset()清空底层所托管的对象
  m_data.reset();
  m_isUnsolicited = false;

  /**
  * typedef time_point TimePoint；
  * using time_point = boost::chrono::time_point<steady_clock>;
  * boost::chrono::time_point<steady_clock> 是使用Boost库中的Chrono组件定义的一个时间点time_point，
  * 它基于一个稳定时钟（steady_clock）。在 Boost Chrono 库中，time_point 是一个模板类，用于表示一个特定的时间点，
  * 而steady_clock是一个时钟类型，它提供了一个稳定的时间来源，不受系统时间变化的影响。
  * 详情请见 https://ndnsim.net/2.7/doxygen/classndn_1_1time_1_1steady__clock.html
  * /
  m_staleTime = time::steady_clock::TimePoint();
}

} // namespace cs
} // namespace nfd
```
