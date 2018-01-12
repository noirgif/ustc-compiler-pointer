# Unique_ptr

接下来，我们介绍 C++ 11 中引入的一种智能指针`std::unique_ptr`。它在很多方面和`auto_ptr`类似，可以说是淘汰`std::auto_ptr`的替代品。它并没有引入新的机制来实现智能内存的管理，仍然用指针来管理对象；同`auto_ptr`一样，它享有对象的独享权，不能用两个`unique_ptr`指向同一个对象。

## unique_ptr 使用

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

## 部分源码分析

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

## unique_ptr 的总结

总体上说，`std::unique_ptr`和`std::auto_ptr`实现的功能是类似的，但是前者使用了 C++11 中的右值引用的特性，使得它们有如下的几点区别：

- auto_ptr 有拷贝语义，拷贝后源对象变得无效；unique_ptr 则无拷贝语义，但提供了移动语义。
- auto_ptr 不可作为容器元素，unique_ptr 可以作为容器元素
- unique_ptr 还可以设置销毁资源的 deleter，这就使得它不仅可以管理内存资源，也可以管理其它的需要释放的资源

