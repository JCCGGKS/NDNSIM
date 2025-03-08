## Packet

具体的介绍可以见[NDN Packet Format Specification](https://docs.named-data.net/NDN-packet-spec/current/intro.html)

NDN中有两中基本的包类型：兴趣包和数据包。

每个NDN packet都以TLV(Type--length--value)格式编码。

## 兴趣包

> Interest =  INTEREST-TYPE TLV-LENGTH
>
> ​		 Name
>
> ​                 [CanBePrefix]
>
> ​		 [MustBeFresh]
>
> ​		 [ForwardingHint]
>
> ​		 [Nonce]
>
> ​		 [InterestLifetime]
>
> ​		 [HopLimit]
>
> ​		 [ApplicationParameters [InterestSignature]]

## 数据包

> Data = DATA-TYPE TLV-LEGTH
>
> ​	    Name
>
> ​	    [MetaInfo]
>
> ​	    [Content]
>
> ​	    DataSignature



### 隐式摘要

在NDN（命名数据网络）中，数据包名称的隐式摘要（Implicit Digest）起着重要的作用。隐式摘要是NDN数据包名称的一部分，它通常位于名称的最后，并且用于确保数据包内容的完整性和可验证性。隐式摘要通过对数据包内容进行哈希计算得到，这样当数据包在网络中传输时，接收方可以通过对比计算得到的摘要与名称中的隐式摘要是否一致来判断数据包是否在传输过程中被篡改。

隐式摘要的作用包括但不限于以下几点：

1. **数据完整性**：通过验证数据包的隐式摘要，可以确保数据在传输过程中未被篡改。
2. **安全性**：隐式摘要提供了一种机制，使得数据包的接收者可以验证数据的来源和完整性。
3. **缓存优化**：在NDN中，缓存是重要的组成部分。隐式摘要可以帮助缓存系统更有效地识别和存储数据包，因为它提供了一种精确匹配数据包内容的方法。
4. **避免循环**：在NDN的路由过程中，隐式摘要有助于检测和避免潜在的循环，因为它为每个数据包提供了一个唯一的标识。

在实际应用中，隐式摘要是NDN数据包名称设计中的一个重要特性，它为NDN提供了一种内建的安全机制，并且对于提高网络的效率和可靠性至关重要。在实现NDN网络时，隐式摘要的计算和验证是数据包处理流程中不可或缺的一部分。
