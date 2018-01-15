# 问题及解答

## 简要介绍智能指针

`std::unique_ptr`独占资源，在指针销毁时释放指向资源。

`std::shared_ptr`共享资源所有权，当指向对象的最后一个指针销毁时释放资源。

`std::weak_ptr`不拥有所有权，需要转化为`std::shared_ptr`才能使用资源。

## 是否好用？性能代价？

好用。

智能指针仅在创建、销毁等时候有性能损失，解引用是没有代价。仅`shared_ptr`有内存代价，并且代价很低。

在[该文档](http://www.open-std.org/jtc1/sc22/wg21/docs/TR18015.pdf)的5.3.1也提到了使用struct，class产生的代价在现在的编译器中已经很低。

在约定使用方面，其实也并没有很麻烦的地方。声明的前缀一般用`typedef`来处理，在操作上，C++ 标准库中的智能指针都做了类似一般指针的重载处理；另外，几个智能指针的基本函数接口（命名，使用）都非常一致。

## 为什么要设计这么多种类的智能指针？

首先，智能指针看似有很多种，但是从设计思想来看，只有两种智能指针（C++ 和 Rust 都是如此）：
独占资源型 和 共享资源型。为了解决共享资源（如 shared_ptr）带来的循环引用的问题，引入了`weak_ptr`来配合使用。

其次，这两种设计思想从需求角度来看都是很容易想到的。