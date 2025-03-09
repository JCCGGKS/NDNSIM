# 类型转换
类型转换大致上可以分为两类：隐式转换(implicit conversion)和显示转换(explicit conversion)。
## implicit conversion
隐式类型转换是自动执行的，无须程序员的介入，有时甚至不需要程序员了解。因此，被称作隐式转换。
发生隐式类型转换的时机：
+ 在大多数表达式中，比int类型小的整型值首先提升为较大的整数类型(比如uint8_t ---> uint16_t)
+ 在条件中，非布尔值转换成布尔类型
+ 初始化过程中，初始值转换成变量的类型；在赋值语句中，右侧运算对象转换成左侧运算对象的类型.
+ 如果算术运算或关系运算的运算对象有多种类型，需要转换成同一种类型。(比如算数运算时，int --->  double)

### 算术转换(arithmetic conversion)
算术转换的规则定义了一套类型转换的层次，其中运算符的运算对象将转换成最宽的类型。
#### 整型提升(integral promotion)
把小整数类型转换成较大的整数类型
+ 对于bool、char、singed char、unsigned char、short和unsigned short等类型，只要它们所有可能的值都能存在int里，就会被提升成int类型；
否则被提升成unsigned int类型。
+ 较大的char类型(wchar_t、char16_t、char32_t)提升成int、unsigned int、long、unsigned long、long long和unsigned long long中最小的一种类型
前提是转换后的类型要能容纳原类型中所有可能的值。
#### 无符号类型的运算对象

### 其它隐式类型转换
#### 数组转换成指针。
数组自动转换成指向数组首元素的指针
```cpp
int arr[10]; //含有10个整数的数组
int* ptr = arr; //arr转换成指向数组首元素的指针相当于&arr[0]
```
PS:需要注意的是，当数组被用作decltype关键字的参数，或者作为取地址符(&)、sizeof以及typeid等运算符的运算对象时，上述转换不会发生。
如果用一个引用初始化数组，上述转换也不会发生。
#### 指针的转换
常量整数值0或者字面值nullptr能转换成任意指针类型
#### 转换成布尔类型
如果指针或算术类型的值为0，转换结果是false；否则转换结果是true。
#### 转换成常量
允许将指向非常量类型的指针转换成指向相应常量类型的指针，对于引用也是这样。
PS：这里的转换，修改的是指针指向内容的常量特性。并且相反的转换并不存在，因为它试图删掉底层的const。
```cpp
int i;
const int &j = i; // 正确
const int *p = &i; // 正确
int &jj=j , *pp=p; // 错误：不允许const转换成非常量
```
#### 类类型定义的转换
类类型能定义由编译器自动执行的转换，不过编译器每次只能执行一种类类型的转换。如果同时提出多个转换请求，这些请求将被拒绝。
如果构造函数只接受一个实参，则它实际上定义了从构造函数的参数类型转换为此类类型的隐式转换机制，有时把这种构造函数称作转换构造函数
```cpp
// class implicit conversion example
#include<iostream>
class A{
public:
  // 转换构造函数
	A(int num) : num_(num){
		std::cout<<this<<"constructor"<<std::endl;
	}
	~A(){
		std::cout<<this<<"destructor"<<std::endl;
	}
private:
	int num_;
};
int main(){
	std::cout<<"implicit conversion:"<<std::endl;
	A a=5;
	std::cout<<"explicit conversion"<<std::endl;
	A a1(5);
}
/**run result
implicit conversion:
0x7ffcaab3d220constructor
explicit conversion
0x7ffcaab3d224constructor
0x7ffcaab3d224destructor
0x7ffcaab3d220destructor
*/
```
类类型转换不总是有效的：如果将构造函数声明为explicit就可以阻止这种隐式转换，只能显示构造
```cpp
#include<iostream>
class A{
public:
	explicit A(int num) : num_(num){
		std::cout<<this<<"constructor"<<std::endl;
	}
	~A(){
		std::cout<<this<<"destructor"<<std::endl;
	}
private:
	int num_;
};
int main(){
	std::cout<<"explicit conversion"<<std::endl;
	A a(5);
	std::cout<<"implicit conversion:"<<std::endl;
	A a1=5;

}
/**run result
error: conversion from ‘int’ to non-scalar type ‘A’ requested
 A a1=5;
*/
```
可以通过运算符重载，将类类型隐式转换为类构造函数参数类型
```cpp
#include<iostream>
class A{
public:
	explicit A(int num) : num_(num){
		std::cout<<this<<"constructor"<<std::endl;
	}
	~A(){
		std::cout<<this<<"destructor"<<std::endl;
	}
	operator int(){
		return num_;
	}
private:
	int num_;
};
int main(){
	std::cout<<"explicit conversion"<<std::endl;
	A a(5);
	std::cout<<"a.num = "<<a<<std::endl;
	//std::cout<<"implicit conversion:"<<std::endl;
	//A a1=5;
}
/**run result
explicit conversion
0x7ffe12659a94constructor
a.num = 5
0x7ffe12659a94destructor
*/
```
## explicit conversion
一个命名的强制类型转换具有如下形式
> cast-name<type>(expression);
其中type是转换的目标类型，expression是要转换的值；
如果type是引用类型，则结果是左值。cast-name有以下四种类型

### static_cast
+ 任何具有明确定义的类型转换，只要不包含底层const，都可以使用static_cast
```cpp
// 将运算符对象强制转换为double类型
int num=5;
double dnum = static_cast<double>(num);
```
+ 当需要把一个较大的算术类型赋值给较小的类型时，static_cast非常有用，此时强制类型会告诉读者和编译器：我们知道并且不在乎潜在的精度损失。
+ static_cast对于编译器无法自动执行的类型转换也非常有用。例如，可以使用static_cast找回存在于void*指针中的值。
```cpp
double d;
void *p = &d; //任何非常量对象的地址都能存入void*
double *dp = static_cast<double*>(p); //将void*转换回初始的指针类型
```
PS：必须确保转换后所得到的类型就是指针所指的类型。类型一旦不符，将产生未定义的后果。
### const_cast
**const_cast**只能改变运算对象的底层const
```cpp
// 只能修改底层const
#include<iostream>
int main(){
        const int const_num=5;
        int num = const_cast<int>(const_num);
        std::cout<<num<<std::endl;
}
/** run result
error: invalid use of const_cast with type ‘int’, which is not a pointer, reference, nor a pointer-to-data-member type
  int num = const_cast<int>(const_num);
*/

#include<iostream>
int main(){
	const int *const_num;
	int *num = const_cast<int*>(const_num);
	std::cout<<"const_num addr: "<<const_num<<std::endl;
	std::cout<<"num addr: "<<num<<std::endl;
}

/** run result
const_num addr: 0x7ffc7e156cc0
num addr: 0x7ffc7e156cc0
*/
```
+ 对于将常量对象转换成非常量对象的行为，一般称其为“去掉const性质(cast away the const)”。
一旦去掉某个对象的const性质，编译器就不再阻止对该对象的写操作了。
+ **如果对象(这里指的是pc指向的对象)本身不是一个常量，使用强制类型转换获得写权限是合法的行为**
  **如果对象是一个常量，再使用const_cast执行写操作就会产生未定义的行为**
```cpp
#include<iostream>
// 对象本身不是一个常量，使用强制类型转换获得写权限是合法的
int main(){
	//The object itself is not a constant
	int val = 5;
	const int *const_num = &val;//can't modify the object by const_num
	int *num = const_cast<int*>(const_num);
	std::cout<<"before modify:"<<std::endl;
	std::cout<<"const_num addr: "<<const_num<<";const_num="<<*const_num<<std::endl;
	std::cout<<"num addr: "<<num<<";num="<<*num<<std::endl;
	*num = 3;
	std::cout<<"after modify:"<<std::endl;
	std::cout<<"const_num addr: "<<const_num<<";const_num="<<*const_num<<std::endl;
	std::cout<<"num addr: "<<num<<";num="<<*num<<std::endl;
}
/** run result
before modify:
const_num addr: 0x7ffcffe95b34;const_num=5
num addr: 0x7ffcffe95b34;num=5
after modify:
const_num addr: 0x7ffcffe95b34;const_num=3
num addr: 0x7ffcffe95b34;num=3
*/
```
```cpp
//对象本身是一个常量，强制类型转换获得写权限不合法
#include<iostream>
int main(){
        //The object itself is  a constant
        const int val = 5;
        const int *const_num = &val;//can't modify the object by const_num
        std::cout<<"before modify:"<<std::endl;

        std::cout<<"const_num addr: "<<const_num<<";const_num="<<*const_num<<std::endl;
        *const_cast<int*>(const_num)=20;
        std::cout<<"after modify:"<<std::endl;
        std::cout<<"const_num addr: "<<const_num<<";const_num="<<*const_num<<std::endl;
}
/** run result
before modify:
const_num addr: 0x7ffe8f41ab9c;const_num=5
after modify:
const_num addr: 0x7ffe8f41ab9c;const_num=20
*/
/**
理论上来说，这段程序不应该运行成功，我问了一下kimi，给出的回复是这样的：
编译器优化：编译器可能不会优化掉对 const 对象的写操作，尤其是在调试模式下。

编译器特定行为：不同的编译器可能对 const_cast 的行为有不同的实现。某些编译器可能没有严格遵循C++标准，或者它们可能在某些情况下对 const 指针和引用的实现方式有所不同。

内存布局：在某些平台上，const 对象可能被存储在可写的内存中，但这并不意味着写入是安全的或符合标准的。

程序状态：即使写入 const 对象没有立即导致程序崩溃，它也可能已经破坏了程序的内部状态，导致不可预测的行为。

运行环境：不同的运行环境（操作系统、硬件等）可能对内存的写入有不同的处理方式，这可能会影响程序的行为。

代码审查和静态分析：在实际的生产环境中，应该使用代码审查和静态分析工具来检测和防止这类未定义行为。
*/
```

+ 只有const_cast能改变表达式的常量属性，使用其它形式的命名强制类型转换改变表达式的常量属性都将引发编译器错误。也不能使用const_cast改变表达式的类型
```cpp
const char *cp;
char *q = static_cast<char*>(cp); //错误，只能使用const_cast强制改变const属性
static_cast<string>(cp); //正确，字符串字面值转换成string类型
const_cast<string>(cp); //错误，const_cast只改变常量属性
```
### reinterpret_cast
通常为运算对象的位模式提供较低层次上的重新解释，举例如下
```cpp
int *ip =;
char *pc = reinterpret_cast<char*>(ip);
```
必须牢记PC所指向的真实对象是一个int而非字符，如果把pc当成普通的字符指针使用，就可能在运行时发生错误。例如
```cpp
#include<iostream>
#include<string>
int main(){
	int val=5;
	int *pn = &val;
	char *pc = reinterpret_cast<char*>(pn);
	std::string str(pc);
	std::cout<<"str = "<<str<<std::endl;
}
/** run result
str =
什么也没有输出
*/
```
WARNING：reinterpret_cast本质上依赖于机器。要想安全地使用reinterpret_cast必须对涉及的类型和编译器实现转换的过程都非常了解。
### 旧式的强制类型转换
```cpp
type(expr); //函数形式的强制类型转换
(type)expr; //c语言风格的强制类型转换
```

## 运行时类型识别(run-time type identification,RTTI)
运行时类型识别的功能由两个运算符实现：
+ typeid运算符，用于返回表达式的类型
+ dynamic_cast运算符，用于将基类的指针或引用安全地转换成派生类的指针或引用。
这两个运算符适用于以下情况：
+ 想使用基类对象的指针或引用执行某个派生类操作并且该操作不是虚函数。一般来说，只要有可能应该尽量使用虚函数。当操作被定义为虚函数时，编译器将根据对象的动态类型自动选择正确的函数版本。
+ 使用RTTI运算符蕴含着更多的潜在风险，必须清楚地知道转换的目标类型并且必须检查类型转换是否成功。
WARNING：使用RTTI必须要加倍小心。在可能的情况下，最好定义虚函数而非直接接管类型管理的重任。
### typeid

### dynamic_cast
使用形式如下：
```cpp
// type必须是一个类类型，并且通常情况下该类型含有虚函数
dynamic_cast<type*>(e); // e必须是一个有效的指针
dynamic_cast<type&>(e); // e必须是一个左值
dynamic_cast<type&&>(e); // e不能是左值
```
