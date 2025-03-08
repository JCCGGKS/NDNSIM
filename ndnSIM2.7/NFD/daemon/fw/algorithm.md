/NFD/daemon/fw/algorithm.hpp
```cpp
/** \brief decide whether Interest can be forwarded to face
 *
 *  \return true if out-record of this face does not exist or has expired,
 *          and there is an in-record not of this face
 *
 *  \note This algorithm has a weakness that it does not permit consumer retransmissions
 *        before out-record expires. Therefore, it's not recommended to use this function
 *        in new strategies.
 *  \todo find a better name for this function
 */
 /**  判断兴趣包是否可以从face转发
 * 如果face对应的out-record不存在或者已经过期并且没有相应的in-record返回true，
 * 该算法有一个缺点，即在out-record过期之前不允许消费者重传。因此，不建议在新策略中使用此函数。
 */
bool canForwardToLegacy(const pit::Entry& pitEntry, const Face& face);
```

/NFD/daemon/fw/algorithm.cpp
```cpp
bool canForwardToLegacy(const pit::Entry& pitEntry, const Face& face){

  // 获取当前的时间
  time::steady_clock::TimePoint now = time::steady_clock::now();
  // 判断face对应的outRecord是否已经过期
  bool hasUnexpiredOutRecord = std::any_of(pitEntry.out_begin(), pitEntry.out_end(),
    [&face, &now] (const pit::OutRecord& outRecord) {
      return &outRecord.getFace() == &face && outRecord.getExpiry() >= now;
    });
  // 没有过期返回false
  if (hasUnexpiredOutRecord) {
    return false;
  }

  bool hasUnexpiredOtherInRecord = std::any_of(pitEntry.in_begin(), pitEntry.in_end(),
    [&face, &now] (const pit::InRecord& inRecord) {
      return &inRecord.getFace() != &face && inRecord.getExpiry() >= now;
    });
  // 有过期的InRecord
  if (!hasUnexpiredOtherInRecord) {
    return false;
  }

  return true;
}
```
