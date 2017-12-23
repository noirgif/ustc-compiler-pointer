# Team 03 Smart Pointers

## Members

| Name | Student ID | Github ID  |
| ---- | ---------- | ---------- |
| 王若晖  | PB15000142 | noirgif    |
| 张一卓  | PB15111610 | eastOffice |
| 钱劲翔  | PB15151800 | Qianjx     |

## Content

智能指针是一种模拟指针的行为，但提供如自动内存管理功能的抽象数据类型。它对普通指针加了一层封装机制，其目的是为了使得智能指针可以方便的管理一个对象的生命期。正确使用智能指针，可以方便的防止指针的悬空引用，内存泄漏等问题。

本次项目尝试从C++的智能指针入手，探索并讨论如下内容：

* raw pointer 在应用中可能产生的问题
* C++ 中 smart pointer 的标准，实现方式和内存管理
* Rust 语言中的 reference 及其内存管理
* smart pointer 在项目中的案例分析

## 1st Commit

### 提交内容及分工情况

王若晖:

张一卓:

- 阅读了C++智能指针的大部分源码，寻找参考文献
- 结合源码总结了 auto_ptr 的使用方式，源码的策略，以及引入了 RAII 的概念
- 完成  [C++ 中的智能指针.md](https://github.com/noirgif/ustc-compiler-pointer/blob/master/C%2B%2B%20%E4%B8%AD%E7%9A%84%E6%99%BA%E8%83%BD%E6%8C%87%E9%92%88.md) 中 `auto_ptr` 的编写

钱劲翔:

- 简单介绍了C++的指针的操作
- 阅读了《C traps and pitfalls》一书指针相关的描述，完成了对C++一般指针存在的悬空指针，野指针等问题的分析，引出设计更加智能的指针的想法
- 完成 [C++ raw_pointer可能引发的问题.md](https://github.com/noirgif/ustc-compiler-pointer/blob/master/C%2B%2B%20raw_pointer%E5%8F%AF%E8%83%BD%E5%BC%95%E5%8F%91%E7%9A%84%E9%97%AE%E9%A2%98.md)

### 下一步计划

- 继续结合源码，向`C++ 中的智能指针.md` 中 unique_ptr, shared_ptr 的分析
- 调研 C++ 标准库以外的智能指针，主要是 boost 库
- 。。。

## Links

[Teamwork Repository](https://github.com/noirgif/ustc-compiler-pointer)

C++ source code of smart pointers：

[auto_ptr](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/source_auto_ptr.cpp), [unique_ptr](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/source_unique_ptr.cpp), [shared_ptr](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/source_shared_ptr.cpp), [weak_ptr](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/source_weak_ptr.cpp)

[ICSE2016-SERVO](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/icse16-servo-preprint.pdf)

