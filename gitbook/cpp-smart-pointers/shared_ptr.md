# Shared_ptr

## 简介

std::shared_ptr是通过指针保持对象共享所有权的智能指针，多个 shared_ptr 对象可占有同一对象。
下列情况之一出现时销毁对象并解分配其内存：
- 最后剩下的占有对象的 shared_ptr 被销毁；
- 最后剩下的占有对象的 shared_ptr 被通过 operator= 或 reset() 赋值为另一指针；
- 用 delete 或在构造期间提供给 shared_ptr 的定制删除器销毁对象。

shared_ptr 能在存储指向一个对象的指针时共享另一对象的所有权。此特性能用于在占有其所属对象时，指向成员对象。存储的指针为 get() 、解引用及比较运算符所访问。被管理指针是在 use_count 抵达零时传递给删除器者。
shared_ptr也可不占有对象，该情况下称它为空 (empty) (空 shared_ptr 可拥有非空存储指针，若以别名使用构造函数创建它)。

## C++中的定义

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

## Shared_ptr的引用计数机制

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

## Shared_ptr的赋值方法

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
  bar = std::make_shared_ptr 
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


## Shared_ptr与unique_ptr,weak_ptr的不同

 1. shared_ptr是引用计数的智能指针，而unique_ptr不是。可以有多个shared_ptr实例指向同一块动态分配的内存，当最后一个shared_ptr离开作用域时，才会释放这块内存。unique_ptr意味着所有权。单个unique_ptr离开作用域时，会立即释放底层内存。

 2. c++默认的智能指针应该是unique_ptr。

 3. weak_ptr是为了解决shared_ptr可能存在的循环计数的问题而提出的一种智能指针，通常与shared_ptr配合使用。如果将一个shared_ptr指针赋值给weak_ptr指针，对shared_ptr指针本身不会造成任何影响。对于weak_ptr指针来说，却可以通过一些方法来探测被赋值过来的shared_ptr指针的有效性，同时weak_ptr指针也可以间接操纵shared_ptr指针。

### Shared_ptr 总结

shared_ptr关键在于共享的对象，只要对象还被引用就不会被释放；这比只能作用单一对象的unique_ptr来说便捷了许多。不过就算有了引用计数机制也不能保证没有内存泄漏，如果出现了循环引用，依然无法有效回收内存，这时候就需要weak_ptr的协助




