# Weak_ptr

`std::weak_ptr`是一种比较特殊的智能指针，它的名字也暗示这一点，它是一种弱指针：弱指针的引用并不能对它的对象进行内存管理。那么，它有什么用呢？

在上文的`std::shared_ptr`中，如果创建两个 shared_ptr 并且让它们互相引用，那么就会产生循环引用，根据引用计数的特性，它们的计数就永远不会为0，导致资源不能释放。weak_ptr 就可以解决这样的问题。

```c++
// 一个循环引用的例子
#include <memory>
#include <iostream>
using namespace std;

class Parent;
class Child;
typedef std::shared_ptr<Parent> parent_ptr;
typedef std::shared_ptr<Child> child_ptr;

class Child
{
public:
    Child()
    {
        cout << "Child ..." << endl;
    }
    ~Child()
    {
        cout << "~Child ..." << endl;
    }
    parent_ptr parent_;
};

class Parent
{
public:
    Parent()
    {
        cout << "Parent ..." << endl;
    }
    ~Parent()
    {
        cout << "~Parent ..." << endl;
    }
    child_ptr child_;
};

int main()
{
    parent_ptr parent(new Parent);
    child_ptr child(new Child);
    parent->child_ = child;
    child->parent_ = parent;

    return 0;
}
// 程序运行结束，但是两个类的析构函数都没有被调用
```

## weak_ptr 使用

1. weak_ptr 的简单定义

   ```c++
       template<typename T> class weak_ptr
       {
       public:
           template <typename Y>
           weak_ptr(const shared_ptr<Y> &r);

           weak_ptr(const weak_ptr &r);			// weak_ptr 只能通过 shared_ptr 或者
         										 // 另一个 weak_ptr 来构造

           template<class Y>
           weak_ptr &operator=( weak_ptr<Y> && r ); 

           template<class Y>
           weak_ptr &operator=(shared_ptr<Y> const &r);
         										// 支持拷贝和赋值，但是不会影响对应的 shared_ptr 										    // 内部对象的计数
   											// 没有重载 *，->，不能操作被管理的资源

           ~weak_ptr();

           bool expired() const;				// 用于判断管理的资源是否已经被释放
           shared_ptr<T> lock() const;			// 用于返回强引用指针：shared_ptr
         	int use_count() const;			    // 用于返回与 shared_ptr 共享的对象的引用计数
       };
   ```

2. weak_ptr 的弱引用机制

   上文的例子中，只需要把 parent 类中的成员改成：

   ```c++
   class Parent
   {
   public:
       std::weak_ptr<parent> child_;
   };
   ```

   就可以不出现循环引用的问题了。那么，weak_ptr 是怎么样做到这一点的呢？

   这就是它的弱引用机制：弱引用仅仅是对在生存期的对象的引用，不进行内存管理，不修改该对象的引用计数。它的构造和析构不会引起引用记数的增加或减少。

   weak_ptr 在功能上类似于普通指针, 然而一个比较大的区别是, 弱引用能检测到所管理的对象是否已经被释放，当被释放时，会自动变成 nullptr ，从而避免访问非法内存。另一方面，它还可以通过 lock 函数来变成 强引用指针，从而操作内存，还不会引起计数的混乱。


