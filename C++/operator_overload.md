# 运算符重载

## operator()
它允许一个类的对象表现得像一个函数
```cpp
#include<iostream>
class MyAdd{
public:
	MyAdd(){std::cout<<"构造函数"<<std::endl;}
	~MyAdd(){std::cout<<"析构函数"<<std::endl;}
	inline int operator()(int a,int b){return a+b;}
};
int main(){
	MyAdd myadd;
	std::cout<<myadd(2,33)<<std::endl;
	return 0;
}

/** run result
构造函数
35
析构函数
*/
```
