## 相关资料

[ndnSIM-2.8 自定义缓存策略-LCD](https://blog.csdn.net/Idontdajiangyou/article/details/122597350)

[ndnSIM-2.1中LCD的简单实现](https://blog.csdn.net/aladeen/article/details/53009975)

# LCD----Leaving Copy Down

## HopCount

### 定义

有关HopCount的一些定义，在路径为`src/ndnSIM/ndn-cxx/ndn-cxx/lp`下的一些文件中，

![](/home/cfq/ndnSIM_learning/ndnSIM2.7/pictures/ndn-cxx_lp.png)

+ tlv.hpp

```cpp
namespace ndn {
namespace lp {
namespace tlv {

/**
 * \brief TLV-TYPE numbers for NDNLPv2
 */
enum {
  ..................
  HopCountTag = 84,
  .....................
};
................

} // namespace tlv
} // namespace lp
} // namespace ndn
```

+ tags.hpp

```cpp
namespace ndn {
namespace lp {

........................

/** \class HopCountTag
 *  \brief a packet tag for HopCount field
 *
 * This tag can be attached to Interest, Data, Nack.
 */
typedef SimpleTag<uint64_t, 0x60000000> HopCountTag;

} // namespace lp
} // namespace ndn
```

+ fields.hpp

```cpp
namespace ndn {
namespace lp {
    
..........................

typedef FieldDecl<field_location_tags::Header,
                  uint64_t,
                  tlv::HopCountTag> HopCountTagField;
BOOST_CONCEPT_ASSERT((Field<HopCountTagField>));

...........

/** \brief Set of all field declarations.
 */
typedef boost::mpl::set<
  .............. ,
  HopCountTagField
  > FieldSet;

} // namespace lp
} // namespace ndn
```

### 修改

对HopCount的修改操作在文件`src/ndnSIM/NFD/daemon/face/generic-link-service.cpp`中

+ encodeLpFields(发送packet的时候要对其进行编码)

```cpp
void
GenericLinkService::encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket)
{
    .....................................

  shared_ptr<lp::HopCountTag> hopCountTag = netPkt.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) {
    lpPacket.add<lp::HopCountTagField>(*hopCountTag);
  }
  else {
    // hopcount的初始值是0
    lpPacket.add<lp::HopCountTagField>(0);
  }
}
```

+ decodeInterest(接收到packet，先对其进行解码)

```cpp
void
GenericLinkService::decodeInterest(const Block& netPkt, const lp::Packet& firstPkt)
{
  .......................

  // forwarding expects Interest to be created with make_shared
  auto interest = make_shared<Interest>(netPkt);

  // Increment HopCount
  if (firstPkt.has<lp::HopCountTagField>()) {
    interest->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  ................................
}
```

+ decodeData

```cpp
void
GenericLinkService::decodeData(const Block& netPkt, const lp::Packet& firstPkt)
{
  BOOST_ASSERT(netPkt.type() == tlv::Data);

  // forwarding expects Data to be created with make_shared
  auto data = make_shared<Data>(netPkt);
  // increment hopcount
  if (firstPkt.has<lp::HopCountTagField>()) {
    data->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  ........................................
}
```

对于缓存节点存储的内容副本回去除HopCount标识，在文件`/home/cfq/ndnSIM2.7/ns3/src/ndnSIM/NFD/daemon/fw/forwarder.cpp`中

```cpp
void
Forwarder::onIncomingData(Face& inFace, const Data& data)
{
.................................

  // 内容副本去除HopCountTag
  
  shared_ptr<Data> dataCopyWithoutTag = make_shared<Data>(data);
  dataCopyWithoutTag->removeTag<lp::HopCountTag>();

  // CS insert
  if (m_csFromNdnSim == nullptr)
    m_cs.insert(*dataCopyWithoutTag);
  else
    m_csFromNdnSim->Add(dataCopyWithoutTag);

  ................................
  // foreach pending downstream
    for (Face* pendingDownstream : pendingDownstreams) {
      if (pendingDownstream->getId() == inFace.getId() &&
          pendingDownstream->getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
      // goto outgoing Data pipeline
      this->onOutgoingData(data, *pendingDownstream);
    }
}
```

打印HopCount，在文件`src/ndnSIM/apps/ndn-consumer.cpp`中

```cpp
void
Consumer::OnData(shared_ptr<const Data> data)
{
  ...................

  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  //如果packet来自本地节点的缓存，则hopCountTag为nullptr
  // local node本地节点指的是用户所在的节点
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  ........................
}
```



## LCDTag

### LCD的基本思想

只在命中节点(缓存节点或者源节点)的下一跳节点进行缓存，即每次缓存都距离用户更近一跳。

### 如何实现

1. 内容副本在缓存中存储之前，会去除HopCount标识，这样一样，不论用户发出的请求是在缓存节点得到响应，还是从源节点得到响应，HopCount都会从0开始计算(从encodeLpFields中可以看出)。
2. 所以在插入缓存之前，先判断当前packet的HopCount是否是1即可。如果是1就是离命中节点只有一跳的下游节点，即可以在该节点进行存储；反之，则直接转发不进行存储。

```cpp
void
Forwarder::onIncomingData(Face& inFace, const Data& data)
{
.................................

  // get HoputCountTag LCD
  shared_ptr<lp::HopCountTag> hopCountTag = data.getTag<lp::HopCountTag>();


  // remove HopCountTag
  shared_ptr<Data> dataCopyWithoutTag = make_shared<Data>(data);
  dataCopyWithoutTag->removeTag<lp::HopCountTag>();

  // hopcount==1 insert
  if(hopCountTag && *hopCountTag == 1){
    // CS insert LCD
    if (m_csFromNdnSim == nullptr)
      m_cs.insert(*dataCopyWithoutTag);
    else
      m_csFromNdnSim->Add(dataCopyWithoutTag);
  }
    ..............
}
```



### 结果演示

#### 网络拓扑

对`ndn-simple`做了改进，增加了两个路由节点

![](/home/cfq/ndnSIM_learning/ndnSIM2.7/pictures/sim_topo.png)

#### 运行方式

```cpp
NS_LOG=ndn-cxx.nfd.ContentStore ./waf --run ndn-simple【使用的缓存策略是`nfd::cs::lru`】
```



#### LCD

![](/home/cfq/ndnSIM_learning/ndnSIM2.7/pictures/cs_lcd.png)

可以发现内容在节点4找到之后只存储下游第一个节点3

#### LCE

![](/home/cfq/ndnSIM_learning/ndnSIM2.7/pictures/cs_lce.png)

可以发现内容在节点4找到之后，存储在下游所有节点3,2,1

#### 日志

更多的日志信息，可以参阅`src/core/model/log.cc`和`src/core/model/log.h`

# MCD----Move Copy Down

MCD在LCD的基础上多了一个删除操作，即从命中节点中删除后存储到下游第一个节点。命中之后，将缓存中的副本删除。

## Cs----新版缓存策略

对函数`  void find(...) const;`进行修改

```

```



## ContentStore----旧版缓存策略

