## 枚举类型
枚举类型(enumeration)可以将一组整型常量组织在一起。
和类一样，每个枚举类型定义了一种新的类型。枚举属于字面值常量类型
C++包含两种枚举：限定作用域的和不限定作用域的。
C++11新标准引入了限定作用域的枚举类型(scoped enumeration)

+ 限定作用域的枚举类型
```cpp
enum class enum_name{
  input,
  output,
  append  
};
// 或者
enum struct enum_name{
  input,
  output,
  append
};
```
+ 不限定作用域的枚举类型
省略关键字class(struct)，枚举类型的名字是可选的
```cpp
// 不限定作用域的枚举类型
enum color{red,yellow,green};
// 未命名的，不限定作用域的枚举类型
enum {floatprec = 6,doubleprec = 10,double_doubleprec=10};
```
如果enum是未命名的，只能在定义该enum时定义对象。

## 枚举成员
+ 限定作用域的枚举类型
枚举成员的名字遵循常规的作用域准则，并且在枚举类型的作用域外是不可访问的。
+ 不限定作用域的枚举类型
枚举成员的作用域与枚举类型本身的作用域相同

```cpp
enum color {red,yellow,green}; // 正确，非限定作用域的枚举类型
enum stoplight {red,yellow,green}; // 错误，重复定义
enum class peppers {red,yellow,green}; // 正确，隐藏枚举成员
color eyes = green; // 不限定作用域枚举类型成员的作用域和类型本身作用域相同
peppers p = green; // green不在作用域
peppers p1 = color::green; // color::green显式访问成员，但是与p1类型不符
color hair = color::red; // color::red显式访问成员
peppers p2 = peppers::red; // peppers::red在作用域
```
默认情况下，枚举值从0开始，依次加1。但是也可以为一个或者多个枚举成员指定专门的值
如果没有显式提供初始值，当前枚举成员的值等于之前枚举成员的值加1
枚举成员是const，因此在初始化枚举成员时提供的初始值必须是常量表达式
+ 可以在任何需要常量表达式的地方使用枚举成员
+ 可以将一个enum作为switch语句的表达式，将枚举值作为case标签

## 和类一样，枚举也定义新的类型
只要enum有名字，就能定义并初始化该类型的成员
```cpp
peppers pp=2； //错误，2不属于类型peppers
pp = peppers::green; //正确，green是peppers的一个枚举成员
```

一个不限定作用域的枚举类型的对象或枚举成员自动地转换成整型
```cpp
int i = color::red; //正确，color::red自动转换成整型
int j = peppers::res; // 错误，限定作用域的类型不会进行隐式转换
```

## 指定enum的大小
尽管每个enum都定义了唯一的类型，但实际上enum是由某种整数类型表示的。
在C++11新标准中，可以在enum的名字后加上冒号以及想在该enum中使用的类型：
```cpp
enum intValue : unsigned long long{
  charTyp = 255, shortTyp = 65535, intTyp = 65535
};
```
如果没有指定enum的潜在类型，默认情况下限定作用域的enum成员类型是int.
对于不限定作用域枚举类型，枚举成员不存在默认类型，只知道成员潜在类型足够大，肯定能够容纳枚举值
如果指定了枚举成员的潜在类型(包括对限定作用域enum的隐式指定)，一旦某个枚举成员的值超出该类型所能容纳的范围，将引发程序错误

## 枚举类型的前置声明
在C++11新标准中，可以提前声明enum。enum的前置声明(无论隐式还是显式)必须指定其成员的大小
```cpp
// 不限定作用域，必须指定成员类型
enum intValues : unsigned long long;
//限定作用域的枚举类型可以使用默认成员类型int
enum class openmodes;
```
