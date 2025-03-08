

# ICN
|information|summarize|
|:------------|:---------------|
|论文<<A survey of information-centric networking research\>>|信息中心网络的研究调查|


# NDN

|information|summarize|
|:------------:|:----------:|
|官网[NDN](https://named-data.net/)||
|[annual progress summaries](https://named-data.net/project/annual-progress-summaries/)|年度进程总结|
|[Consumer-Producer API for Named Data Networking](https://named-data.net/publications/poster_consumer_producer_api/)||
|[A World on NDN: Affordances & Implications of the Named Data Networking Future Internet Architecture](https://named-data.net/publications/techreports/world-on-ndn-11apr2014/)||
|[NDN Documentation](https://docs.named-data.net/)|包括ndn-cxx、NFD、NLSR、Packet Format|
|[Named Data Networking: Motivation & Details](https://named-data.net/project/archoverview/)|命名数据网络：动机与细节，简要介绍NDN|
|论文<<Networking named content\>>||
|论文<<A new approach to securing audio conference tools\>>||
|[Named Data NetWorking Tech Report 001](https://named-data.net/publications/techreports/tr001ndn-proj/)||

# 架构

### Names
|information|summarize|
|:------------:|:----------:|
|论文<<Naming in content-oriented architectures\>>|面向内容架构中的命名(分层结构的名称)|
|论文<<On SDSI's linked local name spaces\>>|关于 SDSI 的链接本地名称空间(SDSI:Simple distributed security infrastructure简单分布式安全基础设施)|
|论文<<A Logic for SDSI's Linked Local Name Spaces\>>|SDSI 链接本地名称空间的逻辑|

### Data-Centric Security
|information|summarize|
|:------------:|:----------:|
|[Deploying Key Management on NDN Testbed](https://named-data.net/publications/techreports/trpublishkey-rev2/)|在 NDN试验平台上部署密钥管理；分层信任模型(密钥命名空间授权使用密钥(携带公钥的包实际上是一个证书，因为它是第三方签署的)来签署特定数据)|
|[Chronos: Serverless Multi-User Chat Over NDN](https://named-data.net/publications/techreports/trchronos/)|信任网络实现通信安全，无需预先商定信任锚点|
|论文<<Interest flooding attack and countermeasures in Named Data Networking\>>|命名数据网络中的兴趣泛洪攻击与对策|

### Routing and Forwarding

|information|summarize|
|:------------:|:----------:|
|[A Case for Stateful Forwarding Plane](https://named-data.net/publications/comcom-stateful-forwarding/)|有状态转发平面案例|
|[Adaptive Forwarding in Named Data Networking](https://named-data.net/publications/p62-yi/)|命名数据网络中的自适应转发|

### In-Network Storage

### Transport Function

## NDN架构开发
### Application research
|information|summarize|
|:------------:|:----------:|
|[NDNVideo: Live and Prerecorded Streaming over NDN](https://named-data.net/publications/techreports/trstreaming/)|Video Streaming视频流|
|[Chronos: Serverless Multi-User Chat Over NDN](https://named-data.net/publications/techreports/trchronos/)|Real-time Conferencing实时会议|
|论文<<Securing instrumented environments over content-centric networking: the case of lighting control and NDN\>>；<<Securing building management systems using named data networking\>>|Building Automation Systems楼宇自动化系统|
|[VANET via Named Data Networking](https://named-data.net/publications/vanet_via_ndn_infocom_nom/)|Vehicular Networking车联网|

现有的NDN[软件平台](http://named-data.net/codebase/platform/)使学生和其他人能够探索基于NDN的分布式文件系统、多用户游戏和网络管理工具。今后几年，将继续开展上述应用的研究工作，并对气候建模和移动健康环境进行新的探索，以推动NDN架构的研究和发展。

为了构建稳健、高效和真正分布式（即无服务器）的点对点NDN应用程序，该架构现在支持名为Sync的新构建模块。Sync([Let’s ChronoSync: Decentralized Dataset State Synchronization in Named Data Networking](https://named-data.net/publications/chronosync/))使用NDN的基本 “兴趣-数据交换 ”通信模型，利用命名约定使多方能够同步其数据集。

### NDN Routing and Forwarding

转发引擎必须支持线速操作，包括可变长度名称的快速查表、存储数百万至数十亿名称的高效数据结构以及快速包处理。项目组成员提出了一种高度可扩展的转发结构和引擎。模拟原型支持以小于10MB的空间存储数百万条目FIB，FIB查找速度达到微秒级。此外，思科和阿尔卡特朗讯的工业团队也开发出了可行的原型路由器。

|information|summarize|
|:------------:|:----------:|
|论文<<Scalable Pending Interest Table design: From principles to practice\>>|可扩展的待定权益表设计： 从原理到实践;可扩展的转发结构和引擎|
|论文<<Scalable NDN Forwarding: Concepts, Issues and Principles\>>|可扩展的NDN转发： 概念、问题和原则；可扩展的转发结构和引擎|
|论文<<Named data networking on a router: Fast and DoS-resistant forwarding with hash tables\>>|思科开发的路由器|
|论文<<A content router for high speed forwarding\>>|阿尔卡特朗讯开发的路由器|



路由协议设计



|information|summarize|
|:------------:|:----------:|
|[OSPFN: An OSPF Based Routing Protocol for Named Data Networking](https://named-data.net/publications/techreports/trospfn/)|基于 OSPF 的命名数据网络路由协议|
|[NLSR: Named-data Link State Routing Protocol](https://named-data.net/publications/nlsr-final/)|命名数据链路状态路由协议|
|[On the Role of Routing in Named Data Networking](https://named-data.net/publications/techreports/tr16-routing/)|论路由选择在命名数据网络中的作用|
|论文<<Sustaining the Internet with hyperbolic mapping\>>|用双曲线映射维持互联网;双曲路由|
|论文<<Hyperbolic geometry of complex networks\>>|复杂网络的双曲几何学|
|论文<<The internet AS-level topology: three data sources and one definitive metric\>>|互联网AS级拓扑：三个数据源和一个确定的度量标准；自治系统的互联网拓扑|




## NDN COMMUNITY & DEPLOYMENT

NDN项目最初使用PARC的开源软件包CCNx作为其代码库。为了提供更灵活的研究开发平台，2013年NDN团队从CCNx分支了一个版本，并在2014年初从头开始实现了一个新的NDN转发器NFD。NFD支持新开发的NDN包格式，并以模块化和可扩展型设计，以促进各种实验。在流行且易于使用的语言(如Python和Javascript)中提供NDN支持，进一步促进了社区的开发活动。

|information|summarize|
|:------------:|:----------:|
|[NFD](http://named-data.net/doc/NFD/current/)|
|[CCNx](http://www.ccnx.org)|
|[NDN.JS: A JavaScript Client Library for Named Data Networking](https://named-data.net/publications/nomen13-ndnjs/)|


# Simulator
|information|summarize|
|:------------:|:----------:|
|[codebase代码库](http://named-data.net/codebase/platform/)|

## ndnSIM
|information|summarize|
|:------------:|:----------:|
|官网[ndnSIM](http://ndnsim.net)|
