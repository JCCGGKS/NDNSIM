# static----类的静态成员
有的时候类需要它的一些成员与类本身直接相关，而不是与类的各个对象保持关联。
## 声明静态成员
+ 通过在成员的声明之前加上关键字`static`使得其与类关联在一起。
+ 和其它成员一样，静态成员可以是`public`也可以是`private`的。
+ 静态数据成员的类型可以是常量、引用、指针、类类型等。
举个例子，定义一个类，用它表示银行的账户记录
```cpp
class Account{
public:
  void calculate(){amount += amount * interestRate;}
  static double rate(){return interestRate;}
  static void rate(double);
private:
  std::string ownet;
  double amount;
  // 声明一个静态数据成员
  static double interestRate;
  static double initRate();
};
```
+ 类的静态成员存在于任何对象之外，对象中不包含任何与静态数据成员有关的数据。
每个Account对象只包含两个数据成员：owner和amount.
interestRate对象被所有Account对象共享
+ 静态成员函数也不与任何对象绑定在一起，它们不包含this指针。
所以，**静态成员函数不能声明成const，也不能在static函数体内使用this指针。**
## 使用类的静态成员
+ 使用作用域运算符直接访问静态成员
```cpp
double r = Account::rate();
```
+ 使用类的对象、引用或者指针访问静态成员
```cpp
Account ac1;
Account *ac2 = &ac1;
r = ac1.rate();
r = ac2->rate();
```
+ 成员函数不用通过作用域运算符就能直接使用静态成员

## 定义静态成员
### 定义静态成员函数
+ 和其它成员函数一样，既可以在类内也可以在类外**定义**静态成员函数。
**当在类外定义静态成员函数时，不能重复static关键字，该关键字只出现在类内部的声明语句**
```cpp
// 类外定义static成员函数
void Account::rate(double newRate){
  interestRate = newRate;
}
```
### 初始化静态数据成员
**由于静态数据成员不属于类的任何一个对象，它们并不是在创建类的对象时被定义的。**
+ 这意味着它们不是由类的构造函数初始化的。
+ 一般来说，不能在类内初始化静态成员，必须在类外定义和初始化每个静态成员。
+ 类似于全局变量，静态数据成员定义在任何函数之外。一旦被定义，将一直存在于程序的整个生命周期。
```cpp
// 类外定义并初始化一个静态数据成员
double Account::interestRate = initRate();
```
**要想确保对象只定义一次，最好的办法就是把静态数据成员的定义和其它非内联函数的定义放在同一个文件**

## 静态成员类内初始化
+ 可以为静态成员提供const整数类型的类内初始值，要求静态成员必须是字面值常量类型constexpr
+ 初始值必须是常量表达式，因为这些成员本身就是常量表达式。
```cpp
class Account{
public:
  ...........
private:
  // 如果period的唯一用途就是定义数组的纬度，不需要在类外定义
  static constexpr int period = 30;//period是常量表达式
  double daily_tbl[period];
};
```
+ <font color='red'>如果某个静态成员的应用场景仅限于编译器可以替换它的值的情况，一个初始化的const或constexpr static不需要在类外定义</font>
+ 如果忽略这个定义，对程序微小的改动可能会造成编译错误，因为程序找不到该数据成员的定义语句；
例如，当需要把Account::period传递给一个接受const int&的函数时，必须定义period

```cpp
// 类内已经初始化，类外无需再次初始化，只定义即可
constexpr Account::period;
```

## 静态成员能用于某些场景，而普通成员不能
+ 静态数据成员可以是不完全类型。静态数据成员的类型可以就是它所属的类类型
+ 非静态数据成员则受到限制，只能声明它所属类的指针或引用。
```cpp
class Bar{
public:
  ....
private:
  static Bar mem1; //正确：静态成员可以是不完全类型
  Bar *mem2; //正确：指针成员可以是不完全类型
  Bar mem3; // 错误：数据成员必须是完全类型
};
```
**什么是不完全类型：声明之后定义之前，也就是此时不知道类的成员都有哪些**

+ 静态数据成员可以作为默认实参，普通成员不可以因为普通成员的值属于对象的一部分
```cpp
class Screen{
public:
  Screen& clear(char = bg);
private:
  static const char bg;
};
```
# operator ++
前缀(prefix)和后缀(postfix)的++运算符都可以用来实现递增操作
## prefix ++
前缀++当作一个表达式使用时，会先递增对象，然后返回递增后的对象。[因此前缀++运算符可以先自增后被赋值]
```cpp
class Counter{
public:
  Counter& operator++(){
    ++value;
    return *this;
  }
private:
  int value;
};
```
## postfix ++
后缀++当作一个表达式使用时，会先返回对象的一个副本(递增之前)，递增对象本身。
```cpp
class Counter{
public:
  Counter operator++(){
    int temp = value;//保留递增之前的副本
    ++value;
    return temp; // 返回递增之前的副本
  }
private:
  int value;
};
```
# overload & override
## overload
重载是指在**同一个作用域**内允许有多个同名函数存在，只要它们的参数列表不同（参数的类型、数量或顺序不同）。重载的函数可以有不同的返回类型，但这不是区分重载函数的主要依据。
## override
覆盖是指在派生类（子类）中重新定义基类（父类）的虚函数（virtual function）。覆盖的函数必须与基类中的虚函数有相同的签名（函数名、参数列表和返回类型）。

# 类
类的基本思想是数据抽象(data abstraction)和封装(encapsulation)。数据抽象是一种依赖于接口(interface)和实现(implementation)分离的编程技术。
类的接口包括用户所能执行的操作，类的实现包括类的数据成员、负责接口实现的函数体以及定义类所需的各种私有函数。

封装实现了类的接口和实现的分离。封装后的类隐藏了实现细节，即类只能访问接口而无法访问实现部分。

类要想实现数据抽象和封装，需要首先定义一个抽象数据类型。在抽象数据类型中，由类的设计者负责考虑类的实现过程；使用该类的程序员只需要抽象地思考类型做了什么
无须了解类型的工作细节。
## 抽象数据类型
### 定义成员函数
所有成员必须在类的内部声明，成员函数体可以定义在类内也可以定义在类外。

#### 引入this
成员函数通过一个名为this的额外的隐式参数访问调用它的那个对象。
当调用一个成员函数时，用请求该函数的对象地址初始化this。例如，如果调用
```cpp
total.isbn();
```
编译器负责把total的地址传递给isbn的隐式形参this，可以等价认为编译器将该调用重写成如下的形式
```cpp
isbn(&isbn);
```

[在成员函数内部，可以直接使用调用该函数的对象的成员，无须通过成员访问符访问，因为this所指的正是这个对象]
**this形参是隐式定义的，任何自定义名为this的参数或变量的行为都是非法的**
**this是一个常量指针，所以修改this里保存的地址是非法的**

默认情况下，this的类型是指向类类型非常量(底层常量)版本的常量指针(顶层常量)，指classtype* const classname  
即不能在一个常量对象上调用普通的成员函数[常量对象，以及常量对象的引用或指针都只能调用常量成员函数，非常量对象既可以调用非常量成员函数也可以调用常量成员函数]


在C++中，this指针的类型取决于它所处的成员函数是否被标记为const;
**非const 成员函数中的this**
在非const成员函数中，this指针不是const的；此时可以通过this指针修改对象的状态
这种情况下，this指针的类型是classname*
**const成员函数中的this**
在const成员函数中，this指针是const的，此时不能通过this指针修改对象的状态。
在这种情况下，this指针的类型const classname*.


#### 类作用域和成员函数
类本身就是一个作用域

编译器分两步处理：首先编译成员的声明，然后才轮到成员函数体(如果有的话)

#### 在类外部定义成员函数

在类外定义成员函数时，成员函数的定义必须与它的声明匹配。[返回类型，参数列表，函数名]


### 定义类相关的非成员函数
如果函数在概念上属于类但是不定义在类中，则它一般应与类声明(而非定义)在同一个头文件内。
在这种方式下，用户使用接口的任何部分都只需要引入一个文件。

### 构造函数
构造函数的任务是初始化类对象的数据成员，无论何时只要类的对象被创建，就会执行构造函数

+ 构造函数的名字和类名相同。构造函数没有返回类型。
+ 构造函数有一个(可能为空的)参数列表和一个(可能为空的)和函数体。
+ 类可以包括多个构造函数(函数重载)
+ 构造函数不能被声明成const。**当创建类的const对象时，直到构造函数完成初始化过程，对象才能真正取得其"常量属性"，在构造过程中可以向其写值**

#### 合成的默认构造函数
默认构造函数无须任何实参。如果类没有显式地定义构造函数，编译器隐式地定义一个默认构造函数。

编译器创建的构造函数被称为合成的默认构造函数[synthesized default constructor]。

#### 某些类不能依赖于合成的默认构造函数
合成的默认构造函数只适合非常简单的类。对于一个普通的类来说，必须定义它自己的默认构造函数。原因如下：
+ 编译器只有在发现类不包含任何构造函数的情况下才会生成一个默认的构造函数。
+ 对于某些类来说合成的默认构造函数可能执行错误的操作。定义在块中的内置类型或复合类型(比如数组和指针)的对象被默认初始化，它们的值是未定义的。
+ 有的时候编译器不能为某些类合成默认的构造函数。例如果类中包含一个其他类类型的成员，且这个成员没有默认构造函数，编译器将无法初始化该成员。

```cpp
classtype() = default;
```
不接受任何实参的构造函数是一个默认构造函数。在C++11标准中，如果需要默认的行为，可以通过在参数列表后面写上`=default`要求编译器生成构造函数(合成默认构造函数)

#### 构造函数初始值列表
```cpp
// 假设有一个类的构造函数如下
Sales_data(const std::string &s):bookNo(s){}
Sales_data(const std::string &s, unsigned n, double p):bookNo(s),units_sold(n),revenue(p*n){}
```

冒号以及冒号和花括号之间的代码成为构造函数初始值列表(constructor initalize list)
当某个数据成员被构造函数初始值列表忽略，会与合成默认构造函数相同的方式隐式初始化

### 拷贝、赋值和析构
+ 拷贝。初始化变量以及以值的方式传递或返回一个对象
+ 赋值。使用了赋值运算符时发生对象的赋值操作
+ 析构。对象不再存在时执行销毁操作

如果不主动定义这些操作，编译器将替我们合成它们，一般来说，编译器生成的版本将对对象的每个成员执行拷贝、赋值和销毁操作。

**某些类不能呢该依赖于合成的版本**
对于某些类来说，合成的版本无法正常工作。特别是当类需要分配类对象之外的资源时，合成的版本常常会失效。

## 访问控制与封装

使用访问说明符(access specifiers)加强类的封装性
+ 定义在public说明符之后的成员在整个程序内可以被访问。
+ 定义在private说明符之后的成员可以被类的成员函数访问，但是不能被使用该类的代码访问，**private部分封装了(即隐藏了)类的实现细节**

### class 与 struct

class与struct唯一的区别在于默认的访问权限不一样

+ struct。定义在第一个访问说明符之前的成员是public
+ class。定义在第一个访问说明符之前的成员是private

### 友元
类可以允许其它类或者函数访问它的非公有成员，方法是令其它类或者函数成为它的友元(friend)
如果类把一个函数作为它的友元，只需要增加一条以friend关键字开始的函数声明语句

```cpp
friend std::istream &read(std::istream&,Sales_data&);
```

友元声明只能出现在类定义的内部，但是在类内出现的具体位置不限。
友元不是类的成员不受它所在区域访问级别的约束。

**一般来说，最好在类定义开始或结束前的 位置集中声明友元**

#### 友元的声明
友元的声明仅仅指定了访问的权限，而非一个通常意义上的函数声明。
如果希望类的用户能够调用某个友元函数，就必须在友元声明之外再专门对函数进行一次声明

为了使友元对类的用户可见，通常把友元声明与类本身放置在同一个头文件中(类的外部)

**许多编译器并未强制限定友元函数在使用之前在类外声明，但是最好提供一个独立的函数声明**

#### 封装的益处
封装有两个重要的优点
+ 确保用户代码不会无意间破坏封装对象的状态
+ 被封装的类的实现细节可以随时改变，无须调整用户级别的代码

**尽管当类的定义发生改变时无须更改用户代码，但是使用了该类的源文件必须重新编译**

## 类的其它特性
### 类成员再探
**定义一个类型成员---pos**
```cpp
// Screen表示显示器中的一个窗口
class Screen{
public:
  typedef std::string::size_type pos;// 自定义某种类型在类中的别名
  // using pos = std::string::size
private:
  pos cursor = 0;// 光标的位置
  pos height = 0, width = 0; //屏幕的高和宽
  std::string contents; //保存Screen内容
};
```
Screen表示显示器中的一个窗口。在Screen的public部分定义了pos，这样用户可以使用这个名字。

类型成员通常出现在类开始的地方

**Screen类的成员函数**

```cpp
public:
  typedef std::string::size_type pos;// 自定义某种类型在类中的别名
  // using pos = std::string::size

  Screen() = default; // 因为Screen有另一个构造函数，所以本函数是必需的

  // cursor被其类内初始值初始化为0
  Screen(pos ht,pos wd,char c):height(ht),width(wd),contents(ht*wd,c){}

  char get()const{
    return contents[cursor];
  } // 读取光标处的字符,隐式内联

  inline char get (pos ht,pos wd)const; //显式内联

  Screen& move(pos r,pos c);// 能在之后被设置为内联



private:
  pos cursor = 0;// 光标的位置
  pos height = 0, width = 0; //屏幕的高和宽
  std::string contents; //保存Screen内容
};
```
#### 内联函数----inline
+ 定义在类内部的成员函数是自动inline的
+ 一些规模较小的函数适用于被声明成内联函数
```cpp
// 在函数的定义处指定inline
inline Screen& Screen::move(pos r,pos c){
  pos row = r * width;
  cursor = row + c;
  return *this;
}

// 在类的内部声明成inline
char Screen::get(pos r,pos c) const{
  pos row = r * width;
  return contents[row+c];
}
```
可以在类的内部把inline作为声明的一部分显式地声明成员函数，同样，也能在类的外部用inline关键字修饰函数的定义


#### 重载成员函数
和非成员函数一样，成员函数也可以被重载，只要函数之间在参数的数量和/或类型上有所区别就行。

#### 可变数据成员
如果想要在const成员函数内，修改类的某个数据成员，可以通过在变量的声明中加入`mutable`关键字做到这一点。
```cpp
class Screen{
public:
  void some_member() const;
private:
  mutable size_t access_ctr; // 即使在一个const对象内也能被修改
};

void Screen::some_member() const{
  ++ access_ctr;
}
```

#### 类数据成员的初始值
定义一个窗口管理类并用它表示显示器上的一组Screen，这个类包含一个Screen类型的vector
默认情况下，希望Window_mgr类开始时总是拥有一个默认初始化的Screen，C++11新标准中，最好的方式就是
把这个默认值声明成一个类内初始值

```cpp
class Window_mgr{
private:
  // Window_mgr追踪的Screen
  // 默认情况下，一个Window_mgr包含一个标准尺寸的空白Screen
  std::vector<Screen> screens{Screen(24,80,' ')};
};
```
在此例中，使用一个单独的元素值对vector成员执行了列表初始化，这个Screen的值被传递给vector<Screen>的构造函数从而创建一个单元素的vector对象



**类内初始值**
```cpp
struct Sales_data{
  std::string bookNo;
  unsigned units_sold = 0;
  double revenue = 0.0；
};
```

C++11新标准规定，可以为数据成员提供一个类内初始值(in-class initializer)。创建对象时，类内初始值将初始化数据成员[units_sold,revenue]
没有初始值的成员将被默认初始化[bookNo将被初始化为空字符串]

当提供一个类内初始值，必须以符号`=`或者花括号表示


### 返回*this的成员函数


### 类类型
每个类定义了唯一的类型。对于两个类来说，即使它们的成员完全一样，这两个类也是不同的类型。

可以把类名作为类型的名字使用，从而直接指向类类型，或者可以把类名跟在关键字class或struct后面
```cpp
Screen scr;
class Screen scr; // 一条等价的声明
```
第二种方式从C语言继承而来，并且在C++语言中也合法
#### 类的声明

仅声明类但暂时不定义
```cpp
class Screen; // Screen类的声明
```

这种声明有时被称作前向声明(forward declaration),它向程序中引入名字Screen并且指明Screen是一种类类型。
对于类型Screen来说，在它声明之后定义之前是一个不完全类型(incomplete type)，即此时只知道Screen是一个类类型，但是不清楚它的数据成员

使用场景
+ 可以定义指向这种类型的指针或引用
+ 也可以声明以不完全类型作为参数或者返回类型的函数(但是不能定义)

对于一个类来说，在创建它的对象之前类必须被定义过，而不能仅仅被声明。否则，编译器无法了解这样的对象需要多少存储空间

```cpp
class Link_screen{
  Screen window;
  Link_screen *next;
  Link_screen *prev;
};
```

### 友元再探
+ 可以把普通的非成员函数定义成友元
+ 可以把其它类定义成友元，也可以把其它类的成员函数定义成友元
+ 友元函数可以定义在类内部，这样的函数是隐式内联的


#### 类之间的友元关系

Window_mgr类的某些成员可能需要访问它管理的Screen类的内部数据。例如，假设需要为Window_mgr添加一个名为clear的成员，
负责把一个指定的Screen的内容都设为空白。为了完成这一任务，clear需要访问Screen的私有成员，要想令这种访问合法，Screen
需要把Window_mgr指定成它的友元：
```cpp
class Screen{
  // Window_mgr的成员可以访问Screen类的私有部分
  friend class Window_mgr;
  // Screen 类的剩余部分
};
```
如果一个类指定了友元类，则友元类的成员函数可以访问此类包括非公有成员在内的所有成员。
Window_mgr的clear成员写成如下的形式
```cpp
class Window_mgr{
public:
  // 窗口中每个屏幕的编号
  using ScreenIndex = std::vector<Screen>::size_type;
  // 按照编号将指定的Screen重置为空白
  void clear(ScreenIndex);
private:
  std::vector<Screen> screens{Screen(24,80,' ')};
};

void Window_mgr::clear(ScreenIndex i){
  // s是一个Screen的引用，指向想清空的那个屏幕
  Screen &s = Screens[i];
  // 将选定的Screen重置为空白
  s.contents = string(s.height*s.width, ' ');
}
```
**友元关系不存在传递性，即如果Window_mgr有自己的友元，这些友元并不能理所当然地具有访问Screen的特权**

#### 令成员函数作为友元


除了令整个Window_mgr作为友元之外，Screen可以只为clear提供访问权限，必须明确指出成员函数所属的类

```cpp
class Screen{
    // Window_mgr::clear必须在Screen类之前被声明
    friend void Window_mgr::clear(ScreenIndex);
    // Screen剩余部分
};
```
#### 函数重载和友元
尽管重载函数的名字相同，但它们仍然是不同的函数。如果一个类想把一组重载函数声明成它的友元，需要对这组函数中的每一个分别声明
```cpp
extern std::ostream& stroeOn(std::ostream& , Screen&);
extern BitMap& storeOn(BitMap& , Screen&);
class Screen{
  // storeOn 的ostream版本能访问Screen对象的私有部分
  friend std::ostream& storeOn(std::ostream&, Screen&);

  // storeOn 的BitMap版本不能访问Screen对象的私有部分
};
```
#### 友元声明和作用域
类和非成员函数的声明不是必须在它们的友元声明之前。当一个名字第一次出现在一个友元声明中时，隐式假定该名字在当前作用域中是可见的，
然而友元本身不一定真的声明在当前作用域中

即使在类的内部定义该函数，也必须在类的外部提供相应的声明从而使得函数可见。
```cpp
struct X{
  friend void f() {/*友元函数可以在类内定义*/}

  X(){f();} // 错误,f还没有被声明
  void g();
  void h();
};
void X::g(){return f();} // 错误:f还没有被声明
void f(); // 声明定义在X中的函数
void X::h(){return f();}//正确：现在f的声明在作用域中了

```
## 类的作用域

#### 作用域和定义在类外部的成员

由于一个类就是一个作用域，所以在类的外部定义成员函数时必须同时提供类名和函数名

```cpp
void Window_mgr::clear(ScreenIndex i){
  Screen &s = screens[i];
  s.contents = string(s.height * s.width,' ');
}
```
+ 编译器在处理参数列表之前已经明确了当前正位于Window_mgr类的作用域中，所以不必再专门说明ScreenIndex、screens是Window_mgr类定义的

+ 函数的返回类型通常出现在函数名之前，因此当成员函数定义在类的外部时，返回类型中使用的名字都位于类的作用域之外。
```cpp
class Window_mgr{
public:
  // 向窗口添加一个Screen，返回它的编号
  ScreenIndex addScreen(const Screen&);

  // 其它成员与之前的一致
};
// 首先处理返回类型，之后才进入Window_mgr作用域

Window_mgr::ScreenIndex Window_mgr::addScreen(const Screen &s){
  screens.push_back(s);
  return screens.size() - 1;
}
```
返回类型出现在类名之前，所以需明确指定哪个类定义了它。

### 名字查找与类的作用域
名字查找(name lookup)(寻找与所用名字最匹配的声明的过程)的过程比较直截了当：
+ 在名字所在块中寻找其声明语句，只考虑在名字的使用之前出现的声明
+ 如果每找到，继续查找外层作用域
+ 如果最终没有找到匹配的声明，程序报错

对于定义在类内部的成员函数来说，解析名字与上述查找规则有所区别。
类的定义分为两步处理：
+ 首先，编译成员的声明
+ 直到类全部可见之后才编译函数体

这种两阶段的方式处理类可以简化类代码的组织方式，因为成员函数体直到整个类可见后才会被处理，所以可以使用类中定义的任何名字。

#### 用于类成员声明的名字查找

以上介绍的两阶段的处理方式只适用于成员函数中使用的名字。
类成员声明中使用的名字(包括返回类型或者参数列表中使用的名字)必须在使用前确保可见
如果某个成员的声明使用了类中尚未出现的名字，编译器会在定义该类的作用域中继续查找，例如
```cpp
typedef double Money;
string bal;
class Account{
public:
  Money balance(){return bal;}
private:
  Money bal;
};
```

编译器看到balance函数的声明语句时。会在Account类的范围内寻找对Money的声明。
编译器只考虑Account中在使用Money前出现的声明，由于找不到匹配的成员，所以编译器会到Account的外层作用域查找，匹配到`typedef double Money`。

[先编译声明后编译函数体]类全部可见之后才会接着编译函数体，所以balance函数体返回的是`Money bal`而不是`string bal`

#### 类型名要特殊处理

```cpp
typedef double Money;
class Account{
public:
  Money balance(){return bal;} // 使用外层作用域的Money
private:
  typedef double Money; // 错误，不能重新定义Money
  Money bal;
};
```

#### 成员定义中的普通块作用域的名字查找


#### 类作用域之后，在外围的作用域中查找


#### 在文件中名字出现处对其进行解析

## 构造函数再探
