# 函数指针 VS 成员函数指针
指向函数的指针可以直接调用，但是函数指针和成员函数指针是不同的
## 函数指针
函数指针声明方式`返回类型(*函数指针名称)(参数列表)`
如果有一个指向普通函数或者静态成员函数的指针，可以直接调用，就像调用一个普通的函数。
```cpp
#include<iostream>
typedef int(*fPtr)(int,int);//fPtr是指向“参数类型为int\int，返回类型为int”的函数指针
int add(int a,int b){
	return a+b;
}
int main(){
	fPtr myf = add;
	std::cout<<myf(3,4)<<std::endl;
	return 0;
}
/*run result
7
*/
```
## 成员函数指针
成员函数指针是指向类中非静态成员函数的指针。
由于成员函数与类的对象或者对象的指针相关联，并且隐含this指针作为第一个参数，它们的调用方式与普通函数指针不一样。
成员函数指针不能直接调用，必须先通过某个对象(使用 ".*") 或者 对象指针(使用“->*”)解引用。

# 类成员指针
成员指针(Pointer to member)是指可以指向类的非静态成员的指针。
一般情况下，指针指向一个对象，但是成员指针指示的是类的成员，而非类的对象。
类的静态成员不属于任何对象，因此无须特殊的指向静态成员的指针，指向静态成员的指针与普通指针没有什么区别

成员指针的类型囊括了类的类型以及成员的类型。当初始化一个这样的指针时，令其指向类的某个成员，但是不指定该成员所属的对象
直到使用成员指针时，才提供成员所属的对象。

解释成员指针之前先定义一个Screen类
```cpp
class Screen{
public:
	typedef std::string::size_type pos;
	char get_cursor() const{return contents[cursor];}
	char get() const;
	char get(pos ht,pos wd) const;
private:
	std::string contents;
	pos cursor;
	pos height,width;
}
```

## 数据成员指针
与声明普通指针不同的是，必须在*之前添加classname::以表示当前定义的指针可以指向classname的成员。
```cpp
// pdata可以指向一个常量(非常量)Screen对象的string成员
const string Screen::*pdata;
```
常量对象的数据成员本身也是常量，因此将指针声明成指向const string成员的指针意味着pdata可以指向
任何Screen对象的一个成员，不管该Screen对象是否是常量。作为交换条件，只能使用pdata读取它所指的成员，而不能向它写入内容


初始化成员指针(或者向它赋值)时，需要指定它所指的成员。
```cpp
// 可以令pdata指向某个非特定Screen对象的contents成员
pdata = &Screen::contents;
```
在C++11新标准中声明成员指针最简单的方式就是使用auto或decltype
```cpp
auto pdata = &Screen::contents;
```

### 使用数据成员指针
成员指针指定了成员而非该成员所属的对象，只有当解引用成员指针时才提供对象的信息。
与成员访问运算符'.'和'->'类似，也有两种成员指针访问运算符'.\*' 和 '->\*'
```cpp
Screen myScreen,*pScreen = &myScreen;
// .*解引用pdata以获得myScreen对象的contents成员
auto s = myScreen.*pdata;

// ->*解引用pdata以获得pScreen对象的contents成员
s = pScreen->*pdata;
```
从概念上说，这些运算符执行两步操作：首先解引用成员指针得到所需的成员。
然后像成员访问运算符一样，通过对象(.\*)或指针(->\*)获取成员。

### 返回数据成员指针的函数


## 成员函数指针

## 将成员函数用作可调用对象
