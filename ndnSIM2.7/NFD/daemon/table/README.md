# Table

NDN中有三个基本的表：

+ Pending Interest Table(PIT)：未决兴趣表。存储请求但未被满足的兴趣，记录兴趣的名称前缀和传入接口。数据包返回时可以根据该传入接口返还给下游请求者。
+ Forwarding Information Base(FIB)：转发信息库。相当于IP中的路由表，为了转发兴趣包，记录兴趣的名称前缀和下一跳接口。
+ Content Store(CS)：内容存储。起到缓存的作用，临时存储经过的数据包，记录数据的名称和具体内容。

![](/home/cfq/ndnSIM_learning/ndnSIM2.7/pictures/forwarding_engine.PNG)