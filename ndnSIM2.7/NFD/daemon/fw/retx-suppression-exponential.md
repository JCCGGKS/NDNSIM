/NFD/daemon/fw/retx-suppression-exponential.cpp
```cpp
RetxSuppressionResult RetxSuppressionExponential::decidePerUpstream(pit::Entry& pitEntry, Face& outFace){

  // NEW if outRecord for the face does not exist
  // 在pitEntry的outRecords寻找outFace
  auto outRecord = pitEntry.getOutRecord(outFace);
  // 没有找到说明是新的outFace
  if (outRecord == pitEntry.out_end()) {
    // 返回NEW标识outFace传出的兴趣是新的
    return RetxSuppressionResult::NEW;
  }

  // 兴趣包最后一次转发的时间
  auto lastOutgoing = outRecord->getLastRenewed();
  // 当前的时间
  auto now = time::steady_clock::now();
  // 最后一次转发距离现在的时间
  auto sinceLastOutgoing = now - lastOutgoing;

  // insertStrategyInfo does not insert m_initialInterval again if it already exists
  PitInfo* pi = outRecord->insertStrategyInfo<PitInfo>(m_initialInterval).first;
  bool shouldSuppress = sinceLastOutgoing < pi->suppressionInterval;

  if (shouldSuppress) {
    // 重传间隔时间太多，抑制
    return RetxSuppressionResult::SUPPRESS;
  }

  // 重传时间达到标准，可以转发
  return RetxSuppressionResult::FORWARD;
}
```
