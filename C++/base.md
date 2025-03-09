@[TOC] C++基础知识
# pointer
+ 常量指针(const pointer)。
1. 顶层const。
2. 这种类型的指针在初始化后不能指向其它地址。
3. 指针本身的值不能被修改，但是它所指向的对象的值可以被修改(如果该对象不是const)
```cpp
int* const ptr;
```
+ 指针常量(Pointer to const)
1. 底层const
2. 这种类型的指针指向一个const对象，意味着通过这个指针不能修改它所指向的对象的值
```cpp
const int* ptr;
```
# static
static变量存储在"静态存储区(static storage area)"。
1. 全局或命名空间作用域的static变量
static变量在程序的整个运行期间都存在，在程序启动时初始化，并在程序结束时销毁。
2. 函数内部的static变量
作用域被限制在声明它们的函数内部，static变量在第一次调用包含它们的函数时初始化，之后的函数调用中会保持上一次调用留下的值。
3. 类成员中的static变量
类中的static变量不属于任何特定的实例对象，而是属于类本身。类的所有对象共享静态成员变量。

**局部static变量的特点：即使是在函数内部声明，也不会在每次函数调用时创建和销毁，它们只会被初始化一次然后一直保持上次用完后的状态**
为更好的理解static，这里举了两个例子作对比：
```cpp
// static vs non-static
#include<iostream>
void mystatic(int in,int incr){
	static int val = 0;
	val+=incr;
	std::cout<<"第"<<in<<"次调用,val="<<val<<std::endl;
}
void mynonstatic(int in,int incr){
	int val = 0;
	val+=incr;
	std::cout<<"第"<<in<<"次调用,val="<<val<<std::endl;
}
int main(){
	std::cout<<"static_example:"<<std::endl;
	for(int i=1;i<=10;++i){
		mystatic(i,i);
	}
	std::cout<<"non_static_example:"<<std::endl;
	for(int i=1;i<=10;++i){
		mynonstatic(i,i);
	}
}
/**run result
static_example:
第1次调用,val=1
第2次调用,val=3
第3次调用,val=6
第4次调用,val=10
第5次调用,val=15
第6次调用,val=21
第7次调用,val=28
第8次调用,val=36
第9次调用,val=45
第10次调用,val=55
non_static_example:
第1次调用,val=1
第2次调用,val=2
第3次调用,val=3
第4次调用,val=4
第5次调用,val=5
第6次调用,val=6
第7次调用,val=7
第8次调用,val=8
第9次调用,val=9
第10次调用,val=10
*/
```


# noncopyable
+ "noncopyable"通常指的是不可复制的特性，意味着某个类的对象不能被复制或赋值。
+ 这通常用于确保对象的生命周期被明确管理，避免意外的复制造成的资源管理问题，如内存泄漏或数据不一致。
Boost库中的实现如下：
```cpp
class noncopyable
  {
  protected:
#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BOOST_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
      BOOST_CONSTEXPR noncopyable() = default;
      ~noncopyable() = default;
#else
      noncopyable() {}
      ~noncopyable() {}
#endif
#if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)
      // 删除拷贝构造函数
      noncopyable( const noncopyable& ) = delete;
      // 删除拷贝赋值函数
      noncopyable& operator=( const noncopyable& ) = delete;
#else
  private:  // emphasize the following members are private
      noncopyable( const noncopyable& );
      noncopyable& operator=( const noncopyable& );
#endif
  };
```
通过删除“拷贝构造和拷贝赋值”函数，实现对象的不可复制

# shared_from_this
shared_from_this是C++标准库std::enable_shared_from_this类的一个成员函数，它允许std::shared_ptr智能指针在对象内部安全地获取和管理对对象本身的引用。
当需要在对象的成员函数中访问或修改对象的std::shared_ptr实例时，这个特性非常有用。

# const VS constexpr
## const
+ const：意味着“我承诺不修改这个值”。编译器负责强制执行const承诺。const声明的值可以在运行时被计算。
## constexpr
+ 主要用于声明常量，作用是把数据置于只读内存区域(更小概率被破坏)，以及提高性能。constexpr的值必须由编译器计算。
常量表达式(const expression)是指值不会改变并且在编译过程中就能得到计算结果的表达式。
+ 字面值属于常量表达式
+ 用常量表达式初始化的const对象也是常量表达式
一个对象(或表达式)是不是常量表达式是由它的数据类型和初始值共同决定的
```cpp
const int max_files = 20; //是常量表达式
const int limit = max_files + 1;//是常量表达式
int staff_size = 27; //不是常量表达式
const int sz = get_size(); //不是常量表达式
```
+ 为了使一个函数可以在常量表达式中使用，这个函数必须被定义为constexpr和consteval，这样才能在编译期表达式中被计算
```cpp
constexpr double square(double x){return x*x;}

constexpr double max1 = 1.4*square(17); // 可行：1.4*square(17)是常量表达式
constexpr double max2 = 1.4*square(var); // 错误：var不是常量，所以square(var)不是常量
const double max3 = 1.4*square(var); // 可行：const声明的变量可以在运行时被计算
```
+ 一个constexpr函数可以输入非常量参数调用，但此时返回值不是常量表达式。只要上下文不需要该函数返回常量表达式，就允许以非常量表达式为参数调用constexpr函数
+ 如果要求某个函数仅在编译时计算，可以声明它为consteval而不是constexpr。
```cpp
consteval double square2(double x) {return x*x;}

constexpr double max4 = 1.4*square2(17); //可行，1.4*square2(17)是常量表达式
constexpr double max5=1.4*square2(var);// 错误，var不是常量
```
+ 被声明为constexpr或consteval的函数是C++版本的纯函数(纯数学函数)。它们只能使用输入参数作为信息，尤其不能修改非局部变量，但可以有循环和自己的局部变量
```cpp
// 求x^n
constexpr double x_n(double x,int n){
	double res=1;
	int i=0;
	while(i<n){
		res*=x;
		++i;
	}
	/*
	for(int i=0;i<n;++i) res*=x;
	*/
	return res;
}
```
# auto VS decltype
## auto类型说明符
编程时常常需要把表达式的值赋给变量，这就要求在声明变量的时候清楚地知道表达式的类型。然而要做到这一点并非那么容易，有时候甚至根本做不到。
C++11新标准引入了auto类型说明符，用它可以让编译器分析表达式所属的类型。和原来那些只对应一种特定类型的说明符(比如double)不同，auto让编译器
通过初始值推算变量的类型。所以auto定义的变量必须有初始值。
```cpp
// 由val1和val2相加的结果推断item的类型
auto item = val1 + val2;// item初始化为val1和val2相加的结果
```
编译器推断出来的auto类型有时候和初始值的类型不完全一样，编译器会适当改变结果类型使其更符合初始化规则。
+ 使用引用其实是使用引用的对象，特别是当引用被用作初始值时，真正参与初始化的其实是引用对象的值。此时编译器以引用对象的类型作为auto的类型
```cpp
int i=0,&r=i;
auto a=r; //a是一个整数
```
+ **auto会忽略顶层const，保留底层const;**
```cpp
int i=0;
const int ci=i,&cr=ci;
auto b =ci; // b:int
auto c = cr; // c : int&
auto d = &i; // d: int*
auto e = &ci;// e: const int*
```
+ 如果希望推断出来的auto类型是一个顶层const，需要明确指出
+ 还可以将引用的类型设置为auto
```cpp
auto &g=ci; // g ： const int&
auto &h=42; //错误：不能为非常量引用绑定字面值
const auto&gg =42;//const int&可以为常量引用绑定字面值
```
**设置一个类型为auto引用时，初始值中的顶层常量属性仍然保留。**
**和往常一样，如果给初始值绑定一个引用，此时的常量就不是顶层常量了**
+ 要在一条语句中定义多个变量，符号“* 和 &”只从属于某个声明符，而非基本数据类型的一部分，因此初始值必须是同一类型
```cpp
auto k=ci,&l=i;// 正确，k和l都是int数据类型；k:int；l:int &；
auto &m=ci,*p=&ci;// 正确，m和p都是const int数据类型；m:const int&；p:const int*
auto &n=i,*p2=&ci;// 错误，n是int数据类型，p2是const int数据类型
```
## decltype类型指示符
有些情况，希望从表达式的类型推断出要定义的变量的类型，但是不想用该表达式的值初始化变量。
为了满足这一要求，C++11新标准引入了第二种类型说明符decltype，它的作用是选择并返回操作数的数据类型
```cpp
decltype(f()) sum;
```
编译器并不实际调用函数f，而是使用当调用发生时f的返回值类型作为sum的类型。
decltype处理顶层const和引用的方式与auto不太一样：
+ 如果decltype使用的表达式是一个变量，则decltype返回该变量的类型(包括顶层const和引用在内)
```cpp
const int ci=0, &cj=ci;
decltype(ci) x=0; // x:const int
decltype(cj) y=x; // y:const int &
decltype(cj) z; // 错误，z是一个const int引用必须要绑定对象
```
有些表达式将向decltype返回一个引用类型。一般来说当这种情况发生时，意味着该表达式的结果对象能作为一条赋值语句的左值
```cpp
int i=42, *p=&i, &r=i;
decltype(r+0) b; // 正确，加法的结果是int，所以b是int类型
// decltype的结果可以是引用类型
decltype(*p) c; // 错误：c是int&，必须初始化
```
+ 因为r是一个引用，因此decltype(r)的结果是引用类型。如果想让结果类型是r所指的类型，可以把r作为表达式的一部分。
+ **如果表达式的内容是解引用操作，则decltype将得到引用类型**

**decltype((variable))的结果永远是引用；decltype(variable)结果只有当variable本身就是一个引用时才是引用**
