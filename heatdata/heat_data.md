## 查找热点数据

### 题目描述

> 给你一个整数数组nums和一个整数k，请返回其中出现频率前k高的元素。可以按照任意顺序返回答案。
>
> + 1 <= nums.length <= 10^5
> + k的取值范围是[1，数组中不相同的元素的个数]
> + 题目数据保证答案是唯一的，即数组中前k个高频元素的集合是唯一的
>
> **示例1**
>
> 输入： nums=[1,1,1,2,2,3]，k=2
>
> 输出：[1,2]\(或者[2,1])
>
> **示例2**
>
> 输入：nums=[1]， k=1
>
> 输出：[1]

### 思路

根据题目中的描述可以得出，数据的热度是根据其访问频率来确定的。所以在寻找热点数据之前需要先对数据的出现次数进行统计，最直接也是最简单的方式就是哈希映射统计数据出现的频次。

+ 对于数值型数据来说。既可以使用数组来统计，也可以使用无序map即unordered_map来统计。[如果给定的数值型数据连续性很差的情况下，使用数组统计会非常耗费空间，比如{1,1,1,10,10,20}，给定的不同的数值个数只有3个但是由于最大的数值是20，所以数组的长度一定要大于20才可以，非常浪费空间]
+ 对于非数值型数据来说。只能使用无序map即unordered_map来统计。

统计好各个数值出现的频次之后，找到频次出现前k的数据有以下两种做法：

+ 对所有数据按照出现的频次freq从大到小排序，取出前k个数据。不同的排序算法对应的时间复杂度如下表所示

![sort](./pictures/sort.PNG)

从表中可以看出，排序最优的时间复杂度是O(nlogn)，取出前k个数据的时间复杂度是O(k)，对于这种方法来说，总的时间复杂度就是O(nlogn)

> 如果k==n的话，对所有数据排序是很划算的。但是如果k<n或者说k<<n，这种情况下对所有数据依照频次进行排序是很不划算的(一个很直观的例子就是从百万条数据中找到排名最高的10条数据，为了找出10条数据对百万条数据都排序是非常没有必要的)，为了优化上述算法，试图只对10条数据进行排序，而不是对所有数据都排许。优化之后的算法如下

+ 小根堆实现的优先级队列。把优先级队列即堆的容量设置为k。如果要找到前k个频次出现最高的数据需要使用小根堆[小根堆的堆顶元素是堆中k个元素里最小的，每次剔除元素时剔除频次最小的元素留下的就是频次较大的元素]。第一步现向堆中添加元素，之后判断堆是否超出容量，若超出则剔除堆顶元素。[如果先剔除元素再插入新的元素，会导致剔除的元素可能不是频次较小的数据，比如对于容量为1的堆，现在堆中的元素是6，新元素是4，如果先剔除元素留下的就是较小的4]

  + > 堆是一种特殊的完全二叉树。堆分为大根堆和小根堆。
    >
    > 大根堆：每个节点的值都不小于其子节点的值；堆顶元素是堆中最大的元素。
    >
    > 小根堆：每个节点的值都不大于其子节点的值；堆顶元素是堆中最小的元素。
    >
    > 堆的操作：
    >
    > + 插入，将一个新元素添加到堆中，并通过调整维护堆的特性。时间复杂度为O(nlogn)
    > + 获取堆顶元素，时间复杂度为O(1)[只是get而非remove]
    > + 删除，移除堆中最大/最小的元素即堆顶元素，并将最后一个元素移到堆顶然后经过调整维护堆的特性，时间复杂度为O(nlogn)

    

#### 排序(sort)+查找

```cpp 
#include<iostream> // std::cout std::cin
#include<vector> // std::vector
#include<algorithm> // std::sort
#include<unordered_map> // std::unordered_map
using namespace std;
typedef pair<int,int> pa;
typedef unordered_map<int,int> ma;
void getHeat(const vector<int>&,ma&); // sum data heat
bool cmp(const pa&, const pa&);// compare function
void print(const vector<pa>&); // print data-freq
int main(){
        unordered_map<int,int> data_heat;
        getHeat({1,1,1,2,2,3},data_heat);
        vector<pa> rdata_heat(data_heat.begin(), data_heat.end());
        // sort before
        cout<<"sort before:"<<endl;
        print(rdata_heat);
        sort(rdata_heat.begin(),rdata_heat.end(),cmp);
        // sort after
        cout<<endl<<"sort after:"<<endl;
        print(rdata_heat);
        return 0;

}

void getHeat(const vector<int>& freqs,ma&data_heat){
        for(const auto&freq:freqs) {
                if(data_heat.count(freq)) ++data_heat[freq];
                else data_heat.emplace(freq,1);
        }
}

bool cmp(const pa&p1, const pa&p2){
        return p1.second > p2.second;
}

void print(const vector<pa>&data_heat){
        // print
        for(const auto&heat:data_heat){
                cout<<"data="<<heat.first<<", freq="<<heat.second<<endl;
        }
}

```

#### 排序(map)+查找

```cpp 
在map中的排序是基于按照key来排序的，所以无法对value直接进行排序，如果想对value进行排序，需要用到vector容器以及sort函数。
```



#### 优先级队列(小根堆)

```cpp
#include<iostream> // std::cout std::cin
#include<vector> // std::vector
#include<queue> // std::priority_queue
#include<unordered_map> //std::unordered_map
using namespace std;
typedef pair<int,int> pa;
// small heap
struct cmp{
	bool operator()(const pa&p1, const pa&p2){
		return p1.second > p2.second;
	}
};
typedef priority_queue<pa,vector<pa>,cmp> pq;
typedef unordered_map<int,int> ma;
void getHeat(const vector<int>&,ma&); // sum data heat
void getHeatData(const ma&,pq&,int);
void copyheat(vector<pa>&,pq&);
void print(const vector<pa>&); // print data-freq
int main(){
    unordered_map<int,int> data_heat;
    getHeat({1,1,1,2,2,3},data_heat);
	pq heatdata;
	getHeatData(data_heat,heatdata,2);
	vector<pa>heat;
	copyheat(heat,heatdata);
	print(heat);
    return 0;
}

void getHeat(const vector<int>& freqs,ma&data_heat){
        for(const auto&freq:freqs) {
                if(data_heat.count(freq)) ++data_heat[freq];
                else data_heat.emplace(freq,1);
        }
}

void getHeatData(const ma&freqs,pq&heat,int k){
	for(const auto&freq:freqs){
		heat.push(freq);
		if(heat.size()>k) heat.pop();
	}
}
void copyheat(vector<pa>&heat,pq&heatdata){
	while(!heatdata.empty()){
		heat.push_back(heatdata.top());
		heatdata.pop();
	}
}
void print(const vector<pa>&data_heat){
        // print
	for(const auto&data:data_heat){
		cout<<"data="<<data.first<<",freq="<<data.second<<endl;
	}
}


```

