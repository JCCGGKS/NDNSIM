/NFD/daemon/fw/forwarder-counters.hpp
```cpp
namespace nfd {

// counters provided by Forwarder
class ForwarderCounters
{
public:
  // 声明在/NFD/core/counter.hpp
  PacketCounter nInInterests;
  PacketCounter nOutInterests;
  PacketCounter nInData;
  PacketCounter nOutData;
  PacketCounter nInNacks;
  PacketCounter nOutNacks;
  PacketCounter nSatisfiedInterests;
  PacketCounter nUnsatisfiedInterests;

  PacketCounter nCsHits;
  PacketCounter nCsMisses;
};

} // namespace nfd

```
