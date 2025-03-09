# std::remove_if
位于头文件<<algorithm>>
## template
```cpp
template<class ForwardIterator , class UnaryPredicate>
ForwardIterator remove_if(ForwardIterator first,ForwardIterator last,UnaryPredicate pred);
```
通过移动为元素分配新值来替换元素(The elements are replaced by move-assigning them their new values)
函数模板的行为相当于：
```cpp
template<class ForwardIterator , class UnaryPredicate>
ForwardIterator remove_if(ForwardIterator first,ForwardIterator last,UnaryPredicate pred){
  ForwardIterator result = first;//元素重新插入的起始位置
  while(first!=last){
    //删除满足条件的，留下不满足条件的
    if(!pred(*first)){
      // 只有当result不等于first时，才将first移动到result
      if(result != first){
        *result = std::move(*first);
      }
      ++result;
    }
    ++first;
  }
  // 所有没有被删除的元素都被移动到前面，返回最后一个没有被删除元素的下一个位置
  // 此时[result,last)的元素都是理论上应该被删除的元素
  return result;
}
```
简单点来说就是，如下的一个题目
给定一个数组nums和一个值val，需要原地移除所有数值等于val的元素。元素的顺序可能发生改变。然后返回nums中与val不同的元素的数量。
假设nums中不等于val的元素数量为k，需要执行以下操作：
* 更改nums数组，使得nums的前k个元素包含不等于val的元素。nums的其余元素和val的大小并不重要。
* 返回k。
示例1：
输入：nums=[3,2,2,3]，val=3
输出：2，nums=[2,2,_,_]
示例2：
输入：nums=[0,1,2,2,3,0,4,2]，val=2
输出：5，nums=[0,1,4,0,3,_,_,_]
输出元素可以以任意顺序返回
该问题的解决代码如下：
```cpp
#include<iostream>
#include<vector>
using std::vector;
int myremove_if(vector<int>&nums,int val){
	int in = 0;
	for(auto &num:nums){
		if(num!=val)
			nums[in++] = num;
	}
	return in;
}
int main(){
	std::cout<<"请输入元素个数n以及要删除的元素val用空格隔开:"<<std::endl;
	int n,val;
	std::cin>>n>>val;
	std::cout<<"请依次输入各个元素用空格隔开:"<<std::endl;
	vector<int>nums(n,0);
	for(int i=0;i<n;++i) std::cin>>nums[i];
	int res = myremove_if(nums,val);
	std::cout<<"删除的元素个数res= "<<res<<std::endl;
	std::cout<<"after delete nums:"<<std::endl;
	for(const auto &num:nums)
		std::cout<<num<<" ";
	std::cout<<std::endl;
	return 0;
}

/**run result
请输入元素个数n以及要删除的元素val用空格隔开:
8 2
请依次输入各个元素用空格隔开:
0 1 2 2 3 0 4 2
删除的元素个数res= 5
after delete nums:
0 1 3 0 4 0 4 2
*/
```

# std::find_if
位于头文件<<algorithm>>
## template
```cpp
template<class InputIterator , class UnaryPredicate>
InputIterator find_if(InputIterator first , InputIterator last , UnaryPredicate pred);
```
查找范围内的元素(Find element in range)
返回一个迭代器，指向范围[first,last)中使得pred为true的第一个元素。如果没有找到这样的元素，返回last
该函数模板的行为相当于(The behavior of this function template is equivalent to)：
```cpp
template<class InputIterator , class UnaryPredicate>
InputIterator find_if(InputIterator first , InputIterator last , UnaryPredicate pred){
  while(first != last){
    if(pred(*first)) return first;
    ++ first;
  }
  return last;
}
```

# std::for_each
## template
```cpp
template <class InputIterator,class Function>
Function for_each(InputIterator first, InputIterator last, Function fn);
```

applies function fn to each of the elements in the range[first, last)
为范围[first,last)内的所有元素应用函数fn
The behavior of this template function is equivalent to
模板函数的行为等效于：
```cpp
template <class InputIterator,class Function>
Function for_each(InputIterator first, InputIterator last, Function fn){
  while(first != last){
    fn(*first);
    ++first;
  }
  return fn; // or since C++11,return move(fn);
}
```

# std::max_element
## template
```cpp
template <class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last);
template <class ForwardIterator, class Compare>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Compare comp);
```

该函数的行为等效于
```cpp
template <class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last){
  if(first == last) return last;
  ForwardIterator largest = first;
  while(++first!=last)
    if (*largest < *first)
      largest = first;
  return largest;
}
// version 2
template <class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Compare comp){
  if(first == last) return last;
  ForwardIterator largest = first;
  while(++first!=last)
    if (comp(*largest,*first))
      largest = first;
  return largest;
}
```
# std::min_element
## template
```cpp
template <class ForwardIterator>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last);
template <class ForwardIterator, class Compare>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last, Compare comp);
```

该函数的行为等效于
```cpp
template <class ForwardIterator>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last){
  if(first == last) return last;
  ForwardIterator smallest = first;

  while(++first != last){
    if(*first < *smallest)
      smallest = first
  }

  return smallest;
}
template <class ForwardIterator, class Compare>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last, Compare comp){
  if(first == last) return last;
  ForwardIterator smallest = first;

  while(++first != last){
    if(comp(*first, *smallest))
      smallest = first
  }

  return smallest;
}
```

#
