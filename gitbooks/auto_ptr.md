# 从 C++ 98 讲起 —— auto_ptr

早在 c++ 98 就有一种智能指针—— auto_ptr，它基本上实现了对内存的自动管理，但是也有非常严重的缺陷，并且在 c++ 11 中被摒弃（虽然它还在标准库中，但是非常不推荐使用它）。在这里仍然要介绍它的原因是，它和 c++ 11 中的智能指针相比，概念和实现都更简单，容易上手，而且它是对智能指针的思想 RAII （Resource Acquisition Is Initialization）的一个实现，可以更好的理解智能指针的设计思路。

## auto_ptr 使用

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

   auto_ptr 只是对 RAII 思想的一种简单实现，我们在后面还将看到一些更高级的技术。


