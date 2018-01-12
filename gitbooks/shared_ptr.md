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

## Shared_ptr 的两种简单实现

在 C++ 标准库中的 `std:shared_ptr`中，使用了引用计数来实现多个指针对同一个对象的共享。为了能更好的理解这个概念，我们用两种常用的方法来简单的实现`shared_ptr`。

在引用计数中，最基本的操作如下：

- 每次创建类的新对象时，初始化指针并将引用计数置为1
- 当对象作为另一对象的副本而创建时，拷贝构造函数拷贝指针并增加与之相应的引用计数 
- 对一个对象进行赋值时，赋值操作符减少左操作数（lhs）所指对象的引用计数（如果引用计数为减至0，则删除对象），并增加右操作数（rhs）所指对象的引用计数
- 调用析构函数时，构造函数减少引用计数（如果引用计数减至0，则删除基础对象）

对引用计数的实现由两种策略，一是引入辅助类，二是使用句柄类。

### 辅助类

```c++
    // 使用辅助类实现引用计数
	template <typename T>
    class SmartPtr;
    
    template <typename T>
    class U_Ptr     //辅助类
    {
    private:
        friend class SmartPtr<T>;     
    
        U_Ptr(T *ptr) :p(ptr), count(1) { }
    
        ~U_Ptr() { delete p; }
        int ref_count;   // 引用计数
    
        T *p;            // 对象指针                                          
    };
    
    template <typename T>
    class SmartPtr   //智能指针类
    {
    public:
        SmartPtr(T *ptr) :rp(new U_Ptr<T>(ptr)) { }     
        SmartPtr(const SmartPtr<T> &sp) :rp(sp.rp) { ++rp->ref_count; }  
        // 和 shared_ptr 一样，重载了 '='
        SmartPtr& operator=(const SmartPtr<T>& rhs) {    
            ++rhs.rp->ref_count;     
            if (--rp->ref_count == 0)    
                delete rp;
            rp = rhs.rp;
            return *this;
        }
    
        T & operator *()        //重载*操作符  
        {
            return *(rp->p);
        }
        T* operator ->()       //重载->操作符  
        {
            return rp->p;
        }
    
        ~SmartPtr() {       
            if (--rp->ref_count == 0)   
                delete rp;
            else 
            std::cout << "ref_count = " << rp->ref_count << std::endl;
        }
    private:
        U_Ptr<T> *rp;  //辅助类对象指针
    };
```

辅助类`U_ptr`的所有成员皆为private，因为它只为智能指针所服务。这个辅助类含有两个数据成员：计数**ref_count**与对象指针。总的来说，就是用**辅助类来以封装使用计数与基础对象指针**。

这种方案的**缺点**是每个含有指针的类的实现代码中都要自己控制引用计数，比较繁琐。特别是当有多个这类指针时，维护引用计数比较困难。

### 句柄类

参考了 C++ Primer 中的实现：

```c++
template<class T> class Handle
{
public:
    Handle(T *p = 0):ptr(p), use(new size_t(1)){}
    T& operator*();
    T* operator->();

    const T& operator*()const;
    const T* operator->()const;

    Handle(const Handle& h):ptr(h.ptr), use(h.use)
    { ++*use; }

    Handle& operator=(const Handle&);
    ~Handle() { rem_ref(); }

private:
    T* ptr;            // 共享的对象指针
    size_t *use;       // ref_count
    void rem_ref()
    {
        if（--*use == 0)
        {
            delete ptr;
            delete use;
        }
    }
};

template<class T>
inline Handle<T>& Handle<T>::operator=(const Handle &rhs)
{
    ++*rhs.use;        //protect against self-assignment
    rem_ref();        //decrement use count and delete pointers if needed
    ptr = rhs.ptr;
    use = rhs.use;
    
    return *this;
}

template<class T>
inline T& Handle<T>::operator*()
{
    if(ptr) return *ptr;
    throw std::runtime_error
        ("dereference of unbound Handle");
}

template<class T>
inline T* Handle<T>::operator->()
{
    if(ptr) return ptr;
    throw std::runtime_error
        ("access through of unbound Handle");
}

template<class T>
inline const T& Handle<T>::operator*()const
{
    if(ptr) return *ptr;
    throw std::runtime_error
        ("dereference of unbound Handle");
}

template<class T>
inline const T* Handle<T>::operator->()const
{
    if(ptr) return ptr;
    throw std::runtime_error
        ("access through of unbound Handle");
}
```

与使用辅助类不同的是，句柄 Handle 只有一个类就实现了引用计数；并且，对于`ref_count`，在辅助类中的成员是 int / size_t 类型的，而在这里，是 int*/ size_t * 类型的，因为要保证多个指针共享对象的引用计数一致。

一个使用句柄类的例子：

```c++
int main()
{
    Handle<int> hp(new int(42));
    {
        //new scope
        Handle<int> hp2 = hp;
        std::cout<< *hp <<"  "<< *hp2 << std::endl;		//  输出两个42
        *hp2 = 10;						   //  修改 hp2 的同时也修改了hp
    }									  //  hp2 退出作用域，但是引用计数没到0，对象不释放
    std::cout<< *hp << std::endl;			//  输出为 10
    return 0;
}
```



## Shared_ptr与unique_ptr,weak_ptr的不同

 1. shared_ptr是引用计数的智能指针，而unique_ptr不是。可以有多个shared_ptr实例指向同一块动态分配的内存，当最后一个shared_ptr离开作用域时，才会释放这块内存。unique_ptr意味着所有权。单个unique_ptr离开作用域时，会立即释放底层内存。

 2. c++默认的智能指针应该是unique_ptr。

 3. weak_ptr是为了解决shared_ptr可能存在的循环计数的问题而提出的一种智能指针，通常与shared_ptr配合使用。如果将一个shared_ptr指针赋值给weak_ptr指针，对shared_ptr指针本身不会造成任何影响。对于weak_ptr指针来说，却可以通过一些方法来探测被赋值过来的shared_ptr指针的有效性，同时weak_ptr指针也可以间接操纵shared_ptr指针。

### Shared_ptr 总结

shared_ptr关键在于共享的对象，只要对象还被引用就不会被释放；这比只能作用单一对象的unique_ptr来说便捷了许多。不过就算有了引用计数机制也不能保证没有内存泄漏，如果出现了循环引用，依然无法有效回收内存，这时候就需要weak_ptr的协助


