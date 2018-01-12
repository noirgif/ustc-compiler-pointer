# C++ boost库中的智能指针

## 引言

 boost是非C++标准库（STL），但boost库中也有着大量关于智能指针的定义和使用。在boost中的智能指针包括 scoped_ptr，scoped_array,shared_ptr, weak_ptr, intrusive_ptr, local_shared_ptr,而且其中的shared_ptr和weak_ptr已经加入了C++11标准。

## 相关指针的介绍

1. scoped_ptr:

    一个scoped_ptr独占一个动态分配的对象。这个类似于unique_ptr，它们都只能处理一个对象，一个unique_ptr不能传递它所包含的对象的所有权到另一个unique_ptr。不过scoped_ptr比起unique_ptr要简单，scoped_ptr只是简单保存和独占一个内存地址。一旦用另一个地址来初始化，这个动态分配的对象将在析构阶段释放。

    - 实例

    ```c++
    #include <boost/scoped_ptr.hpp>
    #include <iostream>
    int main() 
    { 
      boost::scoped_ptr<int> i(new int); 
      *i = 1; 
      *i.get() = 2; 
      i.reset(new int); 
    } 
    ```

2. scoped_array

    这个正如名称中的array，scoped_array是动态分配的数组对象，特点与scoped_ptr一样，都只是保存和占有内存地址。此外它重载了[]运算符，即确实可以像数组一样访问它的元素。

    - 实例

    ```c++
      #include <boost/scoped_array.hpp>
      #include <iostream>
      int main()
      {
        boost::scoped_array<int> i(new int[2]);
        *i.get() = 1;
        i[1] = 2;
        i.reset(new int[3]);
      }
    ```

3. shared_ptr

    见[c++ shared_ptr介绍](https://github.com/noirgif/ustc-compiler-pointer/blob/master/cpp-smart-pointer.md#shared_ptr)

4. weak_ptr 

    见[c++ weak_ptr介绍](https://github.com/noirgif/ustc-compiler-pointer/blob/master/cpp-smart-pointer.md#weak_ptr)

5. intrusive_ptr
  
    类似于shared_ptr，intrusive_ptr也存在引用计数机制，实际上官网上表示，除非shared_ptr不能满足你的需求，不然推荐使用shared_ptr。intrusive_ptr的特别之处在于，它的提供计数函数接口（intrusive_ptr_add_ref和intrusive_ptr_release），要求用户自定义。通常使用instrusive_ptr的情况有：

      - 已经写好了内部引用计数器的代码，而我们又没有时间去重写它(或者不能获得那些代码)。

      - 你需要把 this 当作智能指针来使用。

      - shared_ptr的引用计数器分配严重影响了程序的性能。



6. local_shared_ptr

    local_shared_ptr和shared_ptr几乎一样，唯一不同的是它的引用计数是用非原子操作更新的。因此，一个local_shared_ptr的所有的副本必须驻留在本地的单个线程。

   local_shared_ptr可以转换为shared_ptr，反之亦然。可以从shared_ptr对象创建local_shared_ptr对象，并且创建新的引用计数，也就是说可以两个local_shared_ptr引用相同的对象，但不共享相同的计数，可以安全地由两个不同的线程使用（不用担心，当只有一个local_shared_ptr计数归0的时候，并不会销毁对象）。
