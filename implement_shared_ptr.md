# Shared_ptr 的两种简单实现

在 C++ 标准库中的 `std:shared_ptr`中，使用了引用计数来实现多个指针对同一个对象的共享。为了能更好的理解这个概念，我们用两种常用的方法来简单的实现`shared_ptr`。

在引用计数中，最基本的操作如下：

- 每次创建类的新对象时，初始化指针并将引用计数置为1
- 当对象作为另一对象的副本而创建时，拷贝构造函数拷贝指针并增加与之相应的引用计数 
- 对一个对象进行赋值时，赋值操作符减少左操作数（lhs）所指对象的引用计数（如果引用计数为减至0，则删除对象），并增加右操作数（rhs）所指对象的引用计数
- 调用析构函数时，构造函数减少引用计数（如果引用计数减至0，则删除基础对象）

对引用计数的实现由两种策略，一是引入辅助类，二是使用句柄类。

## 辅助类

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

## 句柄类

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

