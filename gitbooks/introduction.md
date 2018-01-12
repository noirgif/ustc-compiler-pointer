# 引言

在没有接触智能之前，很本能的问题就是 C++ 设计它的动机是什么。我们看如下一段代码：

```c++
ObjectType* ptr = new ObjectType();
ptr->use_function();
delete ptr; // after using ptr
```

我们知道，使用如上的普通指针时，需要在使用完指针指向的对象后删除它。然而，如果我们忘记`delete`，那么会留下一个该指针的悬空引用，很容易导致内存泄漏。

那么，在每一次使用普通指针的时候别忘了`delete`不就行了吗？事实上，要做到这一点是很难的。如下面的情况：

```c++
ObjectType* ptr = new ObjectType();
if(something_not_right())
	throw_exception();	// need delete here
delete ptr;
```

如果 if 条件成立，那么指针将得不到 delete，所以要在 throw_exception() 前也加上delete语句。然而，在较大的工程中，这样的异常会有很多，很难一个一个维护，甚至还有没有捕获的异常发生，就为手动回收指针带来了很大的困难。基于这样的考虑，能够自动管理内存，确定变量生存期的智能指针就应运而生了。


