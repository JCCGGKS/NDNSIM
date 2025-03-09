# 定制操作

## 向算法传递函数
### 谓词
谓词是一个可调用的表达式，其返回结果是一个能用作条件的值。标准库算法所使用的谓词分为两类：
+ 一元谓词(unary predicate)，意味着它们只接受单一参数。
+ 二元谓词(binary predicate)，意味着它们有两个参数。
接受谓词参数的算法对输入序列中的元素调用谓词，因此元素类型必须能转换为谓词的参数类型。
```cpp
bool isShorter(const string &s1,const string &s2){
  return s1.size() < s2.size();
}

sort(words.begin(),words.end(),isShorter);
```
### stable_sort
这种稳定排序算法维持相等元素的原有顺序。
```cpp
// 通过调用stable_sort，可以保持等长元素见的原有顺序
stable_sort(words.begin(),words.end(),isShorter);
```

## lambda表达式
根据算法接受一元谓词还是二元谓词，传递给算法的谓词必须严格接受一个或两个参数。
但是，有时希望进行的操作需要更多的参数，超出算法对谓词的限制。
有这样一个问题：
编写一个函数，令其接受一个string和一个长度，并返回一个bool值表示该string的长度是否大于给定长度，是一件很容易的事情。
但是对于只接受一元谓词的函数来说是非常困难的，为了解决这个问题，需要使用另外一些语言特性：lambda

### 介绍lambda

一个lambda表达式具有如下形式
```cpp
[capture list](parameter list) -> return type {function body}
```
+ capture list(捕获列表)是一个lambda所在函数中定义的局部变量的列表(通常为空)
+ return type、parameter list、function body与任何普通函数一样
  + return type：返回类型
  + parameter list：参数列表
  + function body：函数体
+ 与普通函数不同的是，lambda必须使用尾置返回指定返回类型
**可以忽略参数列表和返回类型，但必须永远包含捕获列表和函数体**
```cpp
// 定义一个可调用对象f，不接受参数
auto f = []{return 42;}
// 调用f
cout<<f()<<endl;
```
lambda的调用方式与普通函数的调用方式相同
如果lambda的函数体包含任何单一return语句之外的内容，且未指定返回类型，则返回void
### 向lambda传递参数
+ 与普通函数不同，lambda不能有默认参数
```cpp
// 与isShorter功能相同的带参数的lambda
[](cosnt string &s1, const string &s2){
  return s1.size() < s2.size();
}
```
空捕获列表表明此lambda不使用它所在函数的任何局部变量
在stable_sort函数中使用该lambda
```cpp
stable_sort(words.begin(),word.end(),[](const string &s1, const string &s2){
  return s1.size() < s2.size();
});
```
### 使用捕获列表

## lambda捕获和返回

当定义一个lambda时，编译器生成一个与lambda对应的新的(未命名的)类类型。大致可以这样理解：
当向一个函数传递一个lambda时，同时定义了一个新类型和该类型的一个对象：传递的参数就是此编译器生成的类类型的未命名对象
类似的，当使用auto定义一个用lambda初始化的变量时，定义了一个从lambda生成的类型的对象

默认情况下，从lambda生成的类都包含一个对应该lambda所捕获的变量的数据成员。类似任何普通类的数据成员，lambda的数据成员也在lambda对象创建时被初始化。

类似参数传递，变量的捕获方式可以是值或者引用

|方式|含义|
|:---:|:----:|
|[]|空捕获列表。lambda不能使用所在函数中的变量。一个lambda只有捕获变量之后才能使用它们。|
|[names]|names是一个逗号分隔的名字列表，这些名字都是lambda所在函数的局部变量。默认情况下，捕获列表中的变量都被拷贝。名字前如果使用&，则采用引用捕获方式。|
|[&]|隐式捕获列表，采用引用捕获方式。lambda体中所使用的来自所在函数的实体都采用引用方式使用|
|[=]|隐式捕获列表，采用值捕获方式。lambda体将拷贝所使用的来自所在函数的实体的值|
|[&,identifer_list]|identifer_list是一个逗号分隔的列表，包含0个或多个来自所在函数的变量。这些变量采用值捕获方式，任何隐式捕获的变量都采用引用方式捕获。identifer_list中的名字前面不能使用&.|
|[=,identifer_list]|identifer_list中的变量都采用引用方式捕获，任何隐式捕获的变量都采用值方式捕获。identifer_list中的名字不能包括this，且这些名字之前必须使用&

**为什么this只能以值的方式捕获？**
因为this指针在成员函数中指向当前对象的实例，而lambda表达式通常用于成员函数中。当
lambda表达式被用于异步操作或作为回调函数时，它可能在对象的生命周期之后仍然存在。
如果this以引用的方式被捕获，那么在对象被销毁之后，lambda表达式可能会尝试访问不存在的对象，将会导致未定义的行为。
### 值捕获

### 引用捕获
### 隐式捕获
### 可变lambda
默认情况下，对于一个值被拷贝的变量，lambda不会改变其值。
如果希望能改变一个被捕获的变量的值，就必须在参数列表首部加上关键字mutable
```cpp
void fcn3(){
  size_t v1 = 42;//局部变量
  // f可以改变它所捕获的变量的值
  auto f = [v1]()mutable{return ++v1;}
  v1 = 0;
  auto j = f();//j为43
}
```
## 参数绑定
用在find_if调用中的lambda比较一个string和一个给定大小。可以很容易编写一个完成同样工作的函数
```cpp
bool check_size(const string &s,string::size_type sz){
  return s.size() >= sz;
}
```
但是不能用这个函数作为find_if的一个参数。为了用check_size代替lambda，必须解决如何向sz形参传递一个参数的问题
### 标准库bind函数
解决向check_size传递一个长度参数的问题，方法是使用一个新的名为bind的标准库函数，定义在头文件functional中。
可以将bind函数看作一个通用的函数适配器，接受一个可调用对象，生成一个新的可调用对象来“适应”原对象的参数列表

```cpp
// 调用的一般形式
auto newCallable =  bind(callable , arg_list);
```
newCallable本身是一个可调用对象，arg_list是一个逗号分隔的参数列表，对应给定的callable的参数。
当调用newcallable时，newCallable会调用callable，并传递给它arg_list中的参数。

arg_list中的参数可能包含形如"_n"的名字，其中n是一个整数。
这些参数是占位符，表示newCallable的参数，它们占据了传递给newCallable的参数的位置。
数值n表示生成的可调用对象中参数的位置：_1为newCallable的第一个参数，_2为第二个参数。

### 绑定check_size的参数
使用bind生成一个可调用check_size的对象
```cpp
// 用一个定值作为其大小参数调用check_size
auto check6 = bind(check_size,_1,6);
// 调用check6
string s("hello");
bool b1 = check6(s); //check6(s)会调用check_size(s,6);
```
用check_size代替lambda
```cpp
find_if(words.begin(), words.end(), bind(check_size,_1,6));
```

### 使用placeholders名字
名字_n都定义在一个名为placeholders的命名空间中，这个命名空间本身定义在std命名空间中。
为了使用这些名字，两个命名空间都要写上
```cpp
using std::placeholders::_1;
```
以下的声明方式可以不需要分别声明每个占位符，这种形式说明所有来自namespace_name的名字都可以在程序中直接使用
```cpp
using namespace namespace_name;
// 例如
using namespace std::placeholders;
```
### bind的参数
可以用bind绑定给定可调用对象中的参数或重新安排其顺序，例如，假定f是一个可调用对象，它有5个参数，下面是对bind的调用
```cpp
auto g = bind(f,a,b,_2,c,_1);
// 生成一个新的可调用对象，有两个参数，分别用占位符_2和_1表示。
// 这个新的可调用对象将它自己的参数作为第三个和第五个参数传递给f。
// f的第一个、第二个、和第四个参数分别被绑定到给定的值a,b,c上
```
传递给g的参数按位置绑定到占位符。即，第一个参数绑定到_1，第二个参数绑定到_2。
当调用g时会将`g(_1,_2)`映射为`f(a,b,_2,c,_1)`；比如`g(x,y)`会调用`f(a,b,y,c,x)`

### 用bind重排参数顺序
用bind颠倒isShorter的含义：
```cpp
// 按照单词长度由短至长排序
sort(words.begin(), words.end(), isShorter);
// 按照单词长度由长至短排序
sort(words.begin(), words.end(),bind(isShorter,_2,_1));
```
### 绑定引用参数
默认情况下，bind的那些不是占位符的参数被拷贝到bind返回的可调用对象中。
有时候需要对有些绑定的参数希望以引用的方式传递，或是要绑定的参数的类型无法拷贝。

例如，为了替换一个用引用方式捕获ostream的lambda
```cpp
// os是一个局部变量，引用一个输出流
// c是一个局部变量，类型为char
for_each(words.begin(), words.end(), [&os,c](const string &s){
  os<< s << c<<std::endl;
});
// 可以编写一个函数来替代
ostream& print(ostream &os, const string &s, char c){
  os<<s<<c<<std::endl;
}
```
如果希望传递给bind一个引用对象，必须使用标准库ref函数
```cpp
for_each(words.begin(), words.end(), bind(print, ref(os), _1, ' '));
```
函数ref返回一个对象，包含给定的引用，此对象是可以拷贝的。
标准库还有一个cref函数，生成一个保存const 引用的类。
函数ref和cref都定义在头文件<<functional>>中

### 向后兼容：参数绑定
旧版本C++提供的绑定函数参数的语言特性限制更多，也更复杂。标准库定义了两个分别名为bind1st和bind2nd的函数。
类似bind,这两个函数接受一个函数作为参数，生成一个新的可调用对象，该对象调用给定函数，并将绑定的参数传递给它。
但是，这些函数分别只能绑定第一个或第二个参数。由于这些函数局限太强，在新标准中已被弃用([[deprecated]])
。所谓被弃用的特性就是在新版本中不再支持。新的C++程序应该使用bind.
