/NFD/daemon/table/pit-out-record.hpp
```cpp
namespace nfd {
namespace pit {

// contains information about an Interest toward an outgoing face
// 包含输出face中有关兴趣包的相关信息
class OutRecord : public FaceRecord
{
public:
  // 显式构造函数
  explicit OutRecord(Face& face);

  /**return last NACK returned by getFace()
   *  A nullptr return value 意味着兴趣包仍未处理或已超时。
   *  A non-null return value 意味着最后一次发出的兴趣包已经被Nacked
   */
  const lp::NackHeader* getIncomingNack() const{
    return m_incomingNack.get();
  }

  /** \brief sets a NACK received from getFace()
   *  \return whether incoming NACK is accepted
   *
   *  This is invoked in incoming NACK pipeline.
   *  An incoming NACK is accepted if its Nonce matches getLastNonce().
   *  If accepted, nack.getHeader() will be copied,
   *  and any pointer previously returned by  .getIncomingNack() .
   */
  bool setIncomingNack(const lp::Nack& nack);

  /** clears last NACK
   *  This is invoked in outgoing Interest pipeline.
   *  This invalidates any pointer previously returned by .getIncomingNack() .
   */
  void clearIncomingNack(){
    m_incomingNack.reset();
  }

private:
  unique_ptr<lp::NackHeader> m_incomingNack;
};

} // namespace pit
} // namespace nfd
```


/NFD/daemon/table/pit-out-record.cpp
```cpp
namespace nfd {
namespace pit {

OutRecord::OutRecord(Face& face)
  : FaceRecord(face){
}

bool OutRecord::setIncomingNack(const lp::Nack& nack){
  if (nack.getInterest().getNonce() != this->getLastNonce()) {
    return false;
  }

  m_incomingNack.reset(new lp::NackHeader(nack.getHeader()));
  return true;
}

} // namespace pit
} // namespace nfd
```
