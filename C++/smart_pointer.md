# shared_ptr
位于头文件<<memory>>
## public member function
### std::shared_ptr::reset
```cpp
(1)
// 对象变为空（就像默认构造的一样
// 放弃对当前对象的所有权，如果指向该对象的shared_ptr实例唯一，则销毁该对象
void reset() noexcept;


// 在所有以下情况下， shared_ptr获取p的所有权，使用次数为1 ；以及可选：分别使用del和/或alloc作为删除器和分配器。
// 此外，调用此函数具有与在其值更改之前调用shared_ptr的析构函数相同的副作用（如果此shared_ptr是唯一的，则包括删除托管对象）。
(2)
template <class U>
void reset (U* p);

(3)
template <class U, class D>
void reset (U* p, D del);

(4)
template <class U, class D, class Alloc>
void reset (U* p, D del, Alloc alloc);
```

**example**
```cpp
#include<iostream>
#include<memory>

int main(){
  std::shared_ptr<int> sp; // empty

  sp.reset(new int); // take ownership of pointer
  *sp = 10;
  std::cout<<*sp<<endl //10

  sp.reset(new int); //deletes managed object, acquires new pointer
  *sp=20;
  std::cout<<*sp<<endl; // 10

  sp.reset(); //empty

  return 0;
}
```
# std::unique_ptr
位于头文件<<memory>>


### 普通指针的不足
+ new 和 new[]的内存需要使用delete 和 delete[]释放
+ 程序员的主观失误，忘了或漏了释放
+ 程序员也不确定何时释放

普通指针的释放
+ 类内的指针，在析构函数中释放
+ C++内置数据类型，如何释放
+ new出来的类，本身如何释放

智能指针的设计思路
+ 智能指针是类模板，在栈上创建智能指针对象
+ 把普通指针交给智能指针对象
+ 智能指针对象过期时，调用析构函数释放普通指针的内存

智能指针的类型
+ auto_ptr是C++98的标准，C++17已经弃用
+ unique_ptr、shared_ptr、weak_ptr是C++11标准的

## unique_ptr
unique_ptr独享它指向的对象，即同时只有一个unique_ptr指向同一个对象，当这个unique_ptr被销毁时，指向的对象也随即被销毁

```cpp
// 头文件
#include <memory>
// 第一个模板参数表示指针指向的数据类型，第二个参数指定删除器，缺省用delete释放资源
template <typename _Tp,typename _Dp = default_delete<_Tp>>
class unique_ptr{
  using pointer = typename __uniq_ptr_impl<_Tp, _Dp>::pointer;
  using element_type = _Tp;
  using deleter_type = _Dp;

  explicit unique_ptr(pointer p) noexcept; // 显示构造函数，不可用于隐式转换
  ~ unique_ptr()noexcept;
  T& operator*() const;// 重载\*操作符
  T* operator->() const noexcept; //重载->操作符
  unique_ptr(const unique_ptr &)=delete;// 禁用拷贝构造函数
  unique_ptr& operator=(const unique_ptr&)=delete;// 禁用拷贝赋值函数
  unique_ptr(unique_ptr &&) noexcept; // 移动构造函数
  unique_ptr& operator=(unique_ptr &&)noexcept; // 移动赋值函数
private:
  // 内置的指针
  pointer ptr;
};
```
