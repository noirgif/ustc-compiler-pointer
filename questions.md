# 问题及解答

## 简要介绍智能指针

`std::unique_ptr`独占资源，在指针销毁时释放指向资源。

`std::shared_ptr`共享资源所有权，当指向对象的最后一个指针销毁时释放资源。

`std::weak_ptr`不拥有所有权，需要转化为`std::shared_ptr`才能使用资源。

## 是否好用？性能代价？

好用。

智能指针仅在创建、销毁等时候有性能损失，解引用是没有代价。仅`shared_ptr`有内存代价，并且代价很低。

在[该文档](http://www.open-std.org/jtc1/sc22/wg21/docs/TR18015.pdf)的5.3.1也提到了使用struct，class产生的代价在现在的编译器中已经很低。
