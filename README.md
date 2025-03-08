# 文档介绍

## heatdata

这里介绍了两种拿到热点数据的方式：

+ 对所有数据排序
+ 堆排序进行优化

## zipf

这里分析了ndnSIM中实现的齐夫分布

## literature

记录了相关文献

+ NDN
+ ndnSIM
+ NFD

## ndnSIM2.7

分析ndnSIM中的源码并给出相应的注释

## ndnSIM网络拓扑生成工具

对于简单场景来说，可以直接定义拓扑结构。对于一些复杂的场景，[ndnSIM官网](https://ndnsim.net/current/)提供了一种更简单的定义方式----用户可读格式的文件。格式如下：

```

# 以'#'开始的行会被忽略
#
# 这个文件应该确切的包含两个部分: router and link。每个都以相应的关键字开头
#
# router部分定义了拓扑节点及其相对位置(--vis可以看到)

# 每一行代表一个路由器，并且包含以下数据
#节点名称	注释	y坐标	  x坐标
# node  comment     yPos    xPos
Node0   NA          3       1
Node1   NA          3       2
Node2   NA          3       3
Node3   NA          2       1
Node4   NA          2       2
Node5   NA          2       3
Node6   NA          1       1
Node7   NA          1       2
Node8   NA          1       3
# node可以是任何字符串。可以使用Names::Find按名称访问节点。

# Link部分定义了节点之间的点对点链接以及这些链接的特征

# 每行采用以下格式，只有前两个指标是必需的，其余的可以省略
# srcNode   dstNode     bandwidth   metric  delay   queue
# bandwidth: 链路带宽
# metric: 路由度量(router metric)
# delay: 链路延迟
# queue:  链路上传输队列的MaxPackets(双向)
Node0       Node1       1Mbps       1       10ms    10
Node0       Node3       1Mbps       1       10ms    10
Node1       Node2       1Mbps       1       10ms    10
Node1       Node4       1Mbps       1       10ms    10
Node2       Node5       1Mbps       1       10ms    10
Node3       Node4       1Mbps       1       10ms    10
Node3       Node6       1Mbps       1       10ms    10
Node4       Node5       1Mbps       1       10ms    10
Node4       Node7       1Mbps       1       10ms    10
Node5       Node8       1Mbps       1       10ms    10
Node6       Node7       1Mbps       1       10ms    10
Node7       Node8       1Mbps       1       10ms    10
```

**router metric**

路由度量是计算机网络中用于确定两个网络节点之间最佳路径的一种量化标准。

它为路由协议提供了一种评估不同路由路径优劣的方法，以便选择最佳的路径转发数据包。

路由度量可以基于多种因素，包括路径长度、可靠性、延迟、带宽、负载和通信成本



## 缓存实验和源码整理

这里记录了与缓存相关的实验和源码

+ LCD：利用HopCountTag实现LCD
+ ndn-splitcaching-master：找到的开源项目：分层缓存