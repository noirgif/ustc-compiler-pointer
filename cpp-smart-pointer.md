# C++ 中的智能指针

## 引子

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

## 从 C++ 98 讲起 —— auto_ptr

早在 c++ 98 就有一种智能指针—— auto_ptr，它基本上实现了对内存的自动管理，但是也有非常严重的缺陷，并且在 c++ 11 中被摒弃（虽然它还在标准库中，但是非常不推荐使用它）。在这里仍然要介绍它的原因是，它和 c++ 11 中的智能指针相比，概念和实现都更简单，容易上手，而且它是对智能指针的思想 RAII （Resource Acquisition Is Initialization）的一个实现，可以更好的理解智能指针的设计思路。

### auto_ptr 使用

1. 声明与构造

   ```c++
   template<class T> class auto_ptr;
   template<> class auto_ptr<void>; 	// 声明

   explicit auto_ptr(X* p = 0);     // 指向 p 管理的对象
   auto_ptr(auto_ptr& r);           // 接管 r 管理的对象，且 r 失去管理权

   template<class Y>                // 针对能隐式转换为 T* 类型的 Y*
   auto_ptr<auto_ptr<Y>& r);

   template<class Y>                // 接管 auto_ptr_ref<Y> 类型的 m 管理的对象
   auto_ptr(auto_ptr_ref<Y> m);   
   ```

2. 析构函数

   ```c++
   ~auto_ptr(); // deprecated
   ```

3. copy 与 assign

   ```c++
   auto_ptr& operator=(auto_ptr& r);    

   template<class Y>                  
   auto_ptr& operator=(auto_ptr<Y>& r);

   auto_ptr& operator=(auto_ptr_ref m); 
   ```

   会让 rhs 失去对象的管理权。

4. 隐式转换

   ```c++
   template<class Y>                // 转换为 auto_ptr_ref<Y>
   operator auto_ptr_ref<Y>();

   template<class Y>               // 转换为 auto_ptr<Y>
   operator auto_ptr<Y>();
   ```

5. 其它函数

   ```c++
   T* get() const;                  // 返回 *this 管理的对象指针

   T& operator*() const;            // 返回 *this 管理的对象 （&）
   T* operator->() const;           // 返回 *this 管理的对象指针 （*）

   void reset(T* p = 0);            // reset *this，让它接管 p 指向对象

   T* release();       			// 返回 *this 管理指针，交出它的管理权，并置nullptr
   ```

   ### auto_ptr copy/assign 策略分析

   auto_ptr 最大的问题就是在copy & assign 的时候会转交管理权（管理权 lhs -> rhs），这样就导致 lhs 变成了 nullptr。举例来说：

   ```c++
   auto_ptr<int> p1(new int(1));
   auto_ptr<int> p2(p1);
   cout << *p1 << endl;

   auto_ptr<int> p3=p1;
   cout << *p1 << endl;
   ```

   这是最明显的例子，两处 p1 在分别copy / assign 后变成了空指针。还有更不容易被发现的情况：

   ```c++
   void func(auto_ptr<int> ap)
   {
   cout << *ap << endl;
   }

   auto_ptr<int> ap(new int(1));
   func(ap);
   cout << *ap1 << endl;
   ```

   当 auto_ptr 作为函数参数**值传递**的时候，会新建一个 auto_ptr 进行**拷贝构造**，此时实参的管理权移交给了形参，当函数返回时，实参已经是空指针了。

   如果引用传递 auto_ptr， 虽然没有上述的问题，但是函数就是一个不安全的过程，它有可能会用赋值等操作再次移交它的管理权。

   所以，auto_ptr 还不能用作函数参数传递。auto_ptr 的所有局限性归根究底都是因为它的 copy / assign 策略导致的，我们不妨看一下它这一部分的源码实现：

   ```c++
    _Myt& operator=(_Myt& _Right)                      
     { // assign compatible _Right (assume pointer)
       reset(_Right.release());
       return (*this);
     }

   template<class _Other>                            
   _Myt& operator=(auto_ptr<_Other>& _Right)
   { // assign compatible _Right (assume pointer)
       reset(_Right.release());
       return (*this);
   }

   _Myt& operator=(auto_ptr_ref<_Ty> _Right)         
   { // assign compatible _Right._Ref (assume pointer)
       _Ty *_Ptr = _Right._Ref;
       _Right._Ref = 0;    // release old
       reset(_Ptr);    // set new
       return (*this);
   }
   ```

   可以很明显的看到，` reset(_Right.release());`和`_Right._Ref = 0;`都解除了 rhs 的管理权。

   ### auto_ptr 对象生命周期的管理—— RAII 

   到这里，其实还没有讲到 auto_ptr 为什么可以自动管理对象的生命周期，可以不用手动 delete 呢？在源码中，没有特殊的成员/机制来管理，仅仅是把对象的指针放入 `_Myptr`中，但是为什么用一个对象进行封装，就可以做到这一点呢？

   这就在于之前提到的 RAII 思想，简单概括就是用对象来管理资源（Use objects to manage resources），它是由 Bjarne Stroustrup 首先提出的。我们知道在函数内部的一些成员是放置在栈空间上的，当函数返回时，这些栈上的局部变量就会立即释放空间。要想确保析构函数在对象释放前被执行，可以把对象放到这个程序的栈上。因为stack winding会保证它们的析构函数都会被执行。RAII就利用了栈里面的变量的这一特点。

   RAII 的一般做法是这样的：在对象构造时获取资源，接着控制对资源的访问使之在对象的生命周期内始终保持有效，最后在对象析构的时候释放资源。借此，我们实际上把管理一份资源的责任托管给了一个存放在栈空间上的局部对象。


## Unique_ptr

接下来，我们介绍 C++ 11 中引入的一种智能指针`std::unique_ptr`。它在很多方面和`auto_ptr`类似，可以说是淘汰`std::auto_ptr`的替代品。它并没有引入新的机制来实现智能内存的管理，仍然用指针来管理对象；同`auto_ptr`一样，它享有对象的独享权，不能用两个`unique_ptr`指向同一个对象。

### unique_ptr 使用

1. 声明与构造

   ```c++
     constexpr unique_ptr();                  // 构造没有管理任何资源的空的指针
     constexpr unique_ptr(nullptr_t);		  // nullptr_t 是 C++11 的新类型，nullptr 是它的字面值

     explicit unique_ptr(pointer p);          // 显式声明，构造管理 p 指向资源的对象

     unique_ptr(pointer p,                    // 构造管理 p 指向资源的对象
         typename conditional<is_reference<Deleter>::value,  // 同时把释放的函数设为 d
         Deleter, const Deleter&>::type d);                 

     unique_ptr(pointer p,                     // 和上一个声明类似
         typename remove_reference<Deleter>::type&& d);     // 主要针对 (3) 的 d 是右值引用类型时的													  // 重载

     unique_ptr(unique_ptr&& u);               // 利用已有对象构造，注意，这是语义移动                       

     template<class U, class E>                // 和上面一样，主要针对隐式转化的构造
     unique_ptr(unique_ptr<U, E>&& u);

     template<class U>                        // 把 auto_ptr 变成 unique_ptr
     unique_ptr(auto_ptr<U>&& u);
   ```

   补充：对第三个语法的解释

   - [conditional](http://en.cppreference.com/w/cpp/types/conditional)：

     template< bool B, class T, class F > struct conditional; 中， 如果 B 是 true, 则定义为 T 类型， 如果 B 是 false, 则定义为 F 类型。


   - [is_reference](http://en.cppreference.com/w/cpp/types/is_reference)：在编译期，判断某个类型是否是引用类型。
   - [remove_reference](http://en.cppreference.com/w/cpp/types/remove_reference)：在编译期移除某个类型的引用符号。如： `remove_reference<int&>::type` 类型就是 `int` 类型。

2. 析构函数

   ```
    ~unique_ptr();
   ```

   如果有管理的资源，那么使用`deleter`销毁管理的资源。

3. copy 与 assign

   ```c++
     unique_ptr& opertor=(unique_ptr&& r);          // 把 r 管理的资源交给 *this,右值引用。
     											 // 如果 *this 本身就有管理的资源，那么先用
     											 // deleter 释放
     template<class U, class E>				   // 针对隐式转换
     unique_ptr& operator=(unique_ptr<U, E>&& r);    

     unique_ptr& operator=(nullptr_t);              // 相当于调用 reset，将管理资源的指针置 nullptr   
   ```

4. 其它函数

   ```c++
     pointer release();                            // 移交出 *this 管理资源的指针。如果 *this 没有管											 //	理资源，则返回 nullptr

     void reset(pointer ptr = pointer());          // 设置 *this 管理 ptr 指向的资源， 如果 *this 本											// 身有管理的资源，则先用 deleter 释放该资源

     void swap(unique_ptr& other);                 // 将 *this 管理的资源和 other管理的资源进行交换

     pointer get() const;                          // 获取 *this 管理资源的指针，如果没有则返回 												// nullptr

     Deleter& get_deleter();                       // 获取 *this 绑定的 deleter
     const Deleter& get_deleter() const;           

     explicit operator bool() const;               // 判断是否 *this 管理有资源

     typename std::add_lvalue_reference<T>::type   // 获取 *this 所管理的对象的左值引用
       operator*() const;
     pointer operator->() const;                   // 获取 *this 所管理的对象的指针
   											// 从这两个重载可以看出这个类表现的像个指针
   ```

### 部分源码分析

1. `unique_ptr`的成员

   ```c++
   private:
       typedef std::tuple<typename _Pointer::type, _Dp>  __tuple_type;      
       __tuple_type                                      _M_t;

   public:
       typedef typename _Pointer::type   pointer;                           
       typedef _Tp                       element_type;                      
       typedef _Dp                       deleter_type;                      
   ```

   成员中的核心就是一个tuple二元组，存放所管理的资源的指针以及 deleter 对象。

   public 的成员有：所管理资源的指针的类型，所管理资源的了类型，以及负责销毁资源的 deleter 的类型。

2. 析构函数的实现

   ```c++
   // Destructor, invokes the deleter if the stored pointer is not null.
     ~unique_ptr()
     {
       auto& __ptr = std::get<0>(_M_t);                 

       if (__ptr != nullptr)                            
         get_deleter()(__ptr);

       __ptr = pointer();                               
     }
   ```

   根据上文可以知道，`get<0>(_M_t)`得到的就是`*this` 管理资源的指针的引用。析构函数中，如果管理资源的指针不为空，那么就调用绑定的`deleter`来销毁管理的资源。最后再重置管理的指针。

   由此可以看到，uniqe_ptr 只要调用了析构函数，它管理的资源一定可以得到释放。

3. copy / assign 的实现

   ```c++
     // Assignment.
     /** @brief Move assignment operator.
      *
      * @param __u  The object to transfer ownership from.
      *
      * Invokes the deleter first if this object owns a pointer.
      */
     unique_ptr& operator=(unique_ptr&& __u)                         
     {
       reset(__u.release());
       get_deleter() = std::forward<deleter_type>(__u.get_deleter());
       return *this;
     }

     /** @brief Assignment from another type.
      *
      * @param __u  The object to transfer ownership from, which owns a
      *             convertible pointer to a non-array object.
      *
      * Invokes the deleter first if this object owns a pointer.
      */
     template<typename _Up, typename _Ep>                             
       typename enable_if< __and_<
       is_convertible<typename unique_ptr<_Up, _Ep>::pointer, pointer>,
       __not_<is_array<_Up>>
       >::value,
         unique_ptr&>::type operator=(unique_ptr<_Up, _Ep>&& __u)
     {
       reset(__u.release());
       get_deleter() = std::forward<_Ep>(__u.get_deleter());
       return *this;
     }

     /// Reset the %unique_ptr to empty,invoking the deleter if necessary.
     unique_ptr& operator=(nullptr_t)                                 
     {
       reset();
       return *this;
     }
   ```

   可以看到，在右值引用中，`reset(__u.release());`这一句话就实现了资源管理权的转换。

   为了进一步说明`unique_ptr`赋值中右值引用和`auto_ptr`赋值中左值引用的区别，从`cppreference`上改了一个小例子：

   ```c++
   #include <iostream>
   #include <memory>

   struct Foo {
       Foo() { std::cout << "Foo\n"; }
       ~Foo() { std::cout << "~Foo\n"; }
   };

   int main() 
   {
       std::unique_ptr<Foo> p1;

       {
           std::cout << "Creating new Foo...\n";
           std::unique_ptr<Foo> p2(new Foo);

           // p1 = p2; // Error ! can't copy unique_ptr
           //unique_ptr& opertor=(unique_ptr&& r);
           p1 = std::move(p2);
           std::cout << "About to leave inner block...\n";

           // Foo instance will continue to live, 
           // despite p2 going out of scope
       }

       // unique_ptr& operator=(nullptr_t);
       std::cout << "Creating new Foo...\n"; 
       std::unique_ptr<Foo> p3(new Foo); 
       std::cout << "Before  p3 = nullptr...\n"; 
       p3 = nullptr;
       std::cout << "After  p3 = nullptr...\n"; 

       std::cout << "About to leave program...\n";
   }
   ```

    `unique_ptr`不能直接使用 `p1 = p2` 这样的语句进行赋值，因为重载 `=`时使用的是右值引用。要先用`std::move(p2)`来移动语义，得到 p2 的右值引用，才可以进行赋值。这就是和`auto_ptr`最大的不同之处。

   上面程序的执行结果：

   ```
       Creating new Foo...
       Foo
       About to leave inner block...
       Creating new Foo...
       Foo
       Before  p3 = nullptr...
       ~Foo
       After  p3 = nullptr...
       About to leave program...
       ~Foo
   ```

   这个例子还体现了两点：当`unique_ptr`过了作用域的时候，它所管理的资源仍然存活；当把 p3 置为 nullptr 的时候，它管理的资源得到了释放。

### unique_ptr 的总结

总体上说，`std::unique_ptr`和`std::auto_ptr`实现的功能是类似的，但是前者使用了 C++11 中的右值引用的特性，使得它们有如下的几点区别：

- auto_ptr 有拷贝语义，拷贝后源对象变得无效；unique_ptr 则无拷贝语义，但提供了移动语义。
- auto_ptr 不可作为容器元素，unique_ptr 可以作为容器元素
- unique_ptr 还可以设置销毁资源的 deleter，这就使得它不仅可以管理内存资源，也可以管理其它的需要释放的资源

## Shared_ptr

### 简介

std::shared_ptr是通过指针保持对象共享所有权的智能指针，多个 shared_ptr 对象可占有同一对象。
下列情况之一出现时销毁对象并解分配其内存：
- 最后剩下的占有对象的 shared_ptr 被销毁；
- 最后剩下的占有对象的 shared_ptr 被通过 operator= 或 reset() 赋值为另一指针；
- 用 delete 或在构造期间提供给 shared_ptr 的定制删除器销毁对象。

shared_ptr 能在存储指向一个对象的指针时共享另一对象的所有权。此特性能用于在占有其所属对象时，指向成员对象。存储的指针为 get() 、解引用及比较运算符所访问。被管理指针是在 use_count 抵达零时传递给删除器者。
shared_ptr也可不占有对象，该情况下称它为空 (empty) (空 shared_ptr 可拥有非空存储指针，若以别名使用构造函数创建它)。

### C++中的定义

shared_ptr需要在memory的头文件下才能调用，它的类型是template< class T > class shared_ptr;

[memory中shared_ptr的定义](https://github.com/noirgif/ustc-compiler-pointer/blob/master/References/source_shared_ptr.cpp)

可以看出：shared_ptr和weak_ptr都由一个父类 _Ptr_base继承得到，_Ptr_base负责接收传入的对象；而shared_ptr内部包含两个指针，一个指向对象ptr，另一个指向控制块(control block)，控制块中包含一个引用计数和可选的deleter、allocator。

- 这是带了deleter和allocator的为_Ux类型对象生成shared_ptr的函数

``` c++
template<class _Ux,
    class _Dx,
    class _Alloc>
        void _Resetp(_Ux *_Px, _Dx _Dt, _Alloc _Ax)
    {    // release, take ownership of _Px, deleter _Dt, allocator _Ax
        typedef _Ref_count_del_alloc<_Ux, _Dx, _Alloc> _Refd;
        typename _Alloc::template rebind<_Refd>::other _Al = _Ax;

        _TRY_BEGIN    // allocate control block and reset
            _Refd *_Ptr = _Al.allocate(1);
        new (_Ptr) _Refd(_Px, _Dt, _Al);
        _Resetp0(_Px, _Ptr);
        _CATCH_ALL    // allocation failed, delete resource
            _Dt(_Px);
        _RERAISE;
        _CATCH_END
    }
```

### shared_ptr的引用计数机制

由于shared_ptr的多个指针共同管理对象的机制，为了达到智能管理的效果，引入引用计数机制。

引用计数指的是，所有管理同一块内存的shared_ptr，都共享一个引用计数器，每当一个shared_ptr被赋值（或拷贝构造）给其它shared_ptr时，这个共享的引用计数器就加1，当一个shared_ptr析构或者被用于管理其它裸指针时，这个引用计数器就减1，如果此时发现引用计数器为0，那么说明它是管理这个指针的最后一个shared_ptr了，于是我们释放指针指向的资源。

- 例如：

``` c++
int main()
{
    std::shared_ptr<int> p1(new int(1));// count(int(1))++
    std::shared_ptr<int> p2 = std::make_shared<int>(2);// count(int(2))++
    p1.reset(new int(3));// count(int(1))-- 析构int(1) 同时count(int(3))++
    std::shared_ptr<int> p3 = p1;//count(int(3))++
    p1.reset();//count(int(3))--
    p3.reset();//count(int(3))--,为0，析构int(3)
}
```

- 而在c++源码中，引用计数由各种各样参数的_Reset方法来维护

``` c++
 template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other)
    {    // release resource and take ownership of _Other._Ptr
        _Reset(_Other._Ptr, _Other._Rep, false);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, const _Static_tag&)
    {    // release resource and take ownership of _Other._Ptr
        _Reset(static_cast<_Elem *>(_Other._Ptr), _Other._Rep);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, const _Dynamic_tag&)
    {    // release resource and take ownership of _Other._Ptr
        _Elem *_Ptr = dynamic_cast<_Elem *>(_Other._Ptr);
        if (_Ptr)
            _Reset(_Ptr, _Other._Rep);
        else
            _Reset();
    }

```

### 赋值方法

  1. 默认构造方法，最多三个参数：对象类型；自定义的delete方法，自定义的allocate方法

``` c++
  std::shared_ptr<int> p1;
  std::shared_ptr<int> p2 (nullptr);
  std::shared_ptr<int> p3 (new int);
  std::shared_ptr<int> p4 (new int, std::default_delete<int>()); 
  std::shared_ptr<int> p5 (p4);
  std::shared_ptr<int> p6 (std::move(p5));
  std::shared_ptr<int> p7 (std::unique_ptr<int>(new int));
  std::shared_ptr<C> obj (new C);
  std::shared_ptr<int> p8 (obj, obj->data);

```

  2. = 和 make_shared 和 move 方法

``` c++ 
std::shared_ptr<int> foo;
std::shared_ptr<int> bar (new int(10));
foo = bar;                 // foo和bar共享一个对象
bar = std::make_shared<int> (20);   // move将bar改变为另一个shared_ptr对象 
std::unique_ptr<int> unique (new int(30));
foo = std::move(unique);  // foo改变为unique_ptr对象，原先的10被回收
```
 
 3. static_pointer_cast 静态类型转换

``` c++
struct A {
  static const char* static_type;
  const char* dynamic_type;
  A() { dynamic_type = static_type; }
};
struct B: A {
  static const char* static_type;
  B() { dynamic_type = static_type; }
};

const char* A::static_type = "class A";
const char* B::static_type = "class B";

int main () {
  std::shared_ptr<A> foo;
  std::shared_ptr<B> bar;

  foo = std::make_shared<A>();
  bar = std::static_pointer_cast<B>(foo);
}
```

 4. dynamic_pointer_cast 动态类型转换

``` c++
struct A {
  static const char* static_type;
  const char* dynamic_type;
  A() { dynamic_type = static_type; }
};
struct B: A {
  static const char* static_type;
  B() { dynamic_type = static_type; }
};

const char* A::static_type = "class A";
const char* B::static_type = "class B";

int main () {
  std::shared_ptr<A> foo;
  std::shared_ptr<B> bar;
  bar = std::make_shared<B>();
  foo = std::dynamic_pointer_cast<A>(bar);
}
```

 5. const_pointer_cast 常量类型转换

``` c++
int main () {
  std::shared_ptr<int> foo;
  std::shared_ptr<const int> bar;
  foo = std::make_shared<int>(10);
  bar = std::const_pointer_cast<const int>(foo);
}
```

### shared_ptr与unique_ptr,weak_ptr的不同

 1. shared_ptr是引用计数的智能指针，而unique_ptr不是。可以有多个shared_ptr实例指向同一块动态分配的内存，当最后一个shared_ptr离开作用域时，才会释放这块内存。unique_ptr意味着所有权。单个unique_ptr离开作用域时，会立即释放底层内存。

 2. c++默认的智能指针应该是unique_ptr。

 3. weak_ptr是为了解决shared_ptr可能存在的循环计数的问题而提出的一种智能指针，通常与shared_ptr配合使用。如果将一个shared_ptr指针赋值给weak_ptr指针，对shared_ptr指针本身不会造成任何影响。对于weak_ptr指针来说，却可以通过一些方法来探测被赋值过来的shared_ptr指针的有效性，同时weak_ptr指针也可以间接操纵shared_ptr指针。

### shared_ptr 总结

shared_ptr关键在于共享的对象，只要对象还被引用就不会被释放；这比只能作用单一对象的unique_ptr来说便捷了许多。不过就算有了引用计数机制也不能保证没有内存泄漏，如果出现了循环引用，依然无法有效回收内存，这时候就需要weak_ptr的协助

## Weak_ptr

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

### weak_ptr 使用

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

   ​
