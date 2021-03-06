# Rust 中的智能指针

Rust在过去的版本中有managed pointer（用~和@表示，类似C++的`std::unique_ptr`和`std::shared_ptr`，在现在的版本中已经被移除，现在rust只有raw pointer和reference。

## 引言：Rust 的不同点

与其他语言相同， Rust 同样使用堆和栈来存储对象。和C/C++语言不同的是，Rust提供了一套机制，使其在内存安全的前提下得到接近C/C++的性能。

1. 编译时检查


Rust 能够处理对空指针、悬垂指针的解引用，但和 Java 等语言在运行时处理不同， Rust 会在编译时检查这些问题并报错。

```rust
{
    let s1 = String::from("hello")
    let s2 = s1; // string "hello"'s ownership transferred to s2
    s1; // error! s1 is not valid
}
```

如上的 Rust 程序会在编译时报错，因为 s1 已经不可用。RAII机制也保证了未初始化的值会被使用。

```cpp
{
    std::string a = "hello";
    auto b = std::move(a); // transferred string "hello" to b
    a; // it's still valid to access a, though it's in an unspecified state
}
```

这段程序在 C++ 中能够运行，但是a的值未知。

2. 手动管理变量生存周期

和 Java 等语言不同，Rust 不使用垃圾收集。变量的存储空间被分配在栈上，其生命周期静态决定的。当出了定义的范围后，变量会自动销毁，其存储空间被释放。

```rust
{
    let a = 3;
}
a; // error: no 'a' in the scope
```

如上的程序会出错，因为a并不在范围内。


## 引用

Rust中由于每个value都拥有一个owner，reference是这样一种机制，它在不夺走值的使用权的同时获得其使用权。

```rust
{
  let s1 = String::from("hello");
  let s2 = s1; // s1's ownership is transferred to s2, since String is not Copy
  let s3 = &s2; // s3 got s2's string, without taking its ownership
}
```

如上的程序由于String并非可复制，因此s2会获取该`String`的所有权，之后将无法使用s1(提示`use of moved value`）。

这是一种借用，当原变量失效时，不能存在任何指向其的引用，以防止悬垂引用的产生，如下的程序会在编译时出错：

```rust
let b;
{
    let a = 3;
    b = &a;
} // error, a out of scope while still borrowed
```

### 对竞态的保护

Rust中，一个值最多同时只能够有一个`&mut T`引用（可更改值的引用）或若干个`&T`引用（只读的引用），这种限制避免了在一处使用引用值前被另一处的程序更改的情况。

### 对悬垂引用的保护

和C++不同，Rust在编译阶段保证引用的安全，一个引用会保证有效性，即使用一个引用时，其指向的一定是一个有效的值。

```rust
fn gen_int() -> &i32
{
  let i1 = 1;
  let i2 = &i1;
  
  i2
} // i1 is released returning a dangling reference

let ii = gen_int(); // error! 
```

如上的程序会在编译时报错。

## `Box<T>`

如其名所提示的，`Box<T>`提供的是对堆上分配内存的最简单实现，创立Box对象时，会分配堆上的空间当Box对象销毁时，会将带有的object一并销毁。

使用`Box::new`可以创建一个指针，用`*`来获得其指向的内容。当a的生命周期结束时，堆上的内存会被释放。

```rust
{
    let a = Box::new(5);
    println!("a is {}", a);
}
```

由于`Box`独占所指向的内存空间，因此`Box`不能复制，但是可以使用`clone`来获得另一个样本。

```rust
{
    let a = Box::new(5);
    let mut b = Box::clone(&a);
    *b = *b + 3;
    println!("A is {}", a);
}

会输出`A is 5`.

`Box::new`会在堆上分配能够保存接受该数据的空间，并返回指向该空间的指针(`std::boxed::Box<T>`)。由于历史原因（继承了pre 1.0的~指针），Box实际类似于基本类型，不需要Deref trait，而是可以直接用`*`解引用，Box在Rust源码中的Deref定义如下（可见[网页](https://doc.rust-lang.org/src/alloc/boxed.rs.html#623)）：

```rust
#[stable(feature = "rust1", since = "1.0.0")]
impl<T: ?Sized> Deref for Box<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &**self
    }
}

#[stable(feature = "rust1", since = "1.0.0")]
impl<T: ?Sized> DerefMut for Box<T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut **self
    }
}
```
由于对一个类实例`x`调用`*`，实际上是调用`*(x.deref())`。但Box的特殊地位使其能够独立于deref trait而对`Box<T>`解引用。

如果对非Box的类使用如上的定义的话，会因为在deref函数中调用自身而导致无限循环，使编译器报错。

另外，在目前的`Rust`的实现中，Box的析构是由编译器完成而非定义。在`alloc/boxed.rs`中`Box`的`Drop`实现为空：

```rust
#[stable(feature = "rust1", since = "1.0.0")]
unsafe impl<#[may_dangle] T: ?Sized> Drop for Box<T> {
    fn drop(&mut self) {
        // FIXME: Do nothing, drop is currently performed by compiler.
    }
}
```

如果`box.rs`内容如下：

```rust
fn main() {
    let a = Box::new(4);
}
```

则生成的LLVM IR 代码为：
```llvm-ir
; box::main
; Function Attrs: uwtable
define internal void @_ZN3box4main17h412e9b3ce7074bf7E() unnamed_addr #1 {
start:
  %a = alloca i32*
; call alloc::heap::exchange_malloc
  %0 = call i8* @_ZN5alloc4heap15exchange_malloc17hbdd98bb5621d389dE(i64 4, i64 4)
  %1 = bitcast i8* %0 to i32*
  store i32 4, i32* %1
  store i32* %1, i32** %a
  br label %bb1

bb1:                                              ; preds = %start
; call core::ptr::drop_in_place
  call void @_ZN4core3ptr13drop_in_place17h7cd262323c9f371cE(i32** %a) 
  br label %bb2

bb2:                                              ; preds = %bb1
  ret void
}
```

而在该析构函数中：

```llvm-ir
; core::ptr::drop_in_place
; Function Attrs: uwtable
define internal void @_ZN4core3ptr13drop_in_place17h7cd262323c9f371cE(i32**) unnamed_addr #1 personality i32 (i32, i32, i64, %"unwind::libunwind::_Unwind_Exception"*, %"unwind::libunwind::_Unwind_Context"*)* @rust_eh_personality {
start:
  %personalityslot = alloca { i8*, i32 }
  br label %bb3

bb1:                                              ; preds = %bb3
  ret void

bb2:                                              ; preds = %bb4
  %1 = load { i8*, i32 }, { i8*, i32 }* %personalityslot
  resume { i8*, i32 } %1

bb3:                                              ; preds = %start
  %2 = load i32*, i32** %0, !nonnull !3
; call alloc::heap::box_free
  call void @_ZN5alloc4heap8box_free17hdf1d9b7042aa931eE(i32* %2) 
  br label %bb1

bb4:                                              ; No predecessors!
  %3 = load i32*, i32** %0, !nonnull !3
; call alloc::heap::box_free
  call void @_ZN5alloc4heap8box_free17hdf1d9b7042aa931eE(i32* %3) #7
  br label %bb2
}
```

调用了`alloc::heap::box_free`，该函数负责将其堆上的空间释放：

```rust
#[cfg_attr(not(test), lang = "box_free")]
#[inline]
pub(crate) unsafe fn box_free<T: ?Sized>(ptr: *mut T) {
    let size = size_of_val(&*ptr);
    let align = min_align_of_val(&*ptr);
    // We do not allocate for Box<T> when T is ZST, so deallocation is also not necessary.
    if size != 0 {
        let layout = Layout::from_size_align_unchecked(size, align);
        Heap.dealloc(ptr as *mut u8, layout);
    }
}
```

## `Rc<T>`

`Box<T>`拥有对数据的所有权，因此同一个数据最多只能有一个box指向该数据。

```rust
let x = Box::new(5);
let y = x;
println!("{}", x); // error here
```

原因和之前的`String`相同，`Box`为了保证对数据的所有权，因此不能复制。

这里就有`Rc<T>`来满足这种需求。rc表示reference counted。这和C的`std::shared_ptr`类似，每一个指向该数据rc会给计数器加1，当计数器归0时，该数据被释放。

`Rc`位于`std::rc`下，可以在程序中使用`use std::rc::Rc;`来方便使用。

```rust
  let b;
  {
    let a = Rc::new(5);
    b = Rc::clone(&a);
  }
  println!("a is {}", b); // prints a is 5
```

在`rc.rs`中Rc是这样定义的：

```rust
struct RcBox<T: ?Sized> {
    strong: Cell<usize>,
    weak: Cell<usize>,
    value: T,
}


pub struct Rc<T: ?Sized> {
    ptr: Shared<RcBox<T>>,
    phantom: PhantomData<T>,
}

```

`ptr::Shared`是对Rust中`*mut T`(类似于C中的`T*`,mut表示可变)的封装，本身不做内存管理。

`PhantomData`使得`Rc<T>`的行为和`T`实例的行为一致，即可以调用其指向对象的方法，为了不和`Rc`自身的方法冲突，应该使用类似`Rc::clone(&a)`而不是`a.clone()`的形式调用`Rc`的方法。

```rust
struct St {
}

impl St {
    fn hello(&self) {
        println!("Hello world!");
    }
}

fn main() {
    let a = Rc::new(St {});
    a.hello(); // calls St::hello(&a)
}
```


和C++的`shared_ptr`类似的，RcBox封装该数据和其strong引用和weak引用的计数值，Rc为指向该RcBox的指针，当clone时，strong引用的计数加1.

```rust
impl<T> Rc<T> {
    #[inline]
    fn clone(&self) -> Rc<T> {
        self.inc_strong();
        Rc { ptr: self.ptr }
    }
}

trait RcBoxPtr<T: ?Sized> {
    #[inline]
    fn inc_strong(&self) {
        self.inner().strong.set(self.strong().checked_add(1).unwrap_or_else(|| unsafe { abort() }));
    }
}

```

当`Rc<T>`对象销毁时，会调用drop（类似C++的析构函数），将对应的计数器减1，当计数器归0时，销毁对应RcBox的内容。因为该`drop_in_place`函数并不回收RcBox的空间（仅调用了`T`类型的`drop`，如果有)，因此weak和strong元素仍然可用。

```rust
#[stable(feature = "rust1", since = "1.0.0")]
unsafe impl<#[may_dangle] T: ?Sized> Drop for Rc<T> {
    fn drop(&mut self) {
        unsafe {
            let ptr = self.ptr.as_ptr();

            self.dec_strong();
            if self.strong() == 0 {
                // destroy the contained object
                ptr::drop_in_place(self.ptr.as_mut());

                // remove the implicit "strong weak" pointer now that we've
                // destroyed the contents.
                self.dec_weak();

                if self.weak() == 0 {
                    Heap.dealloc(ptr as *mut u8, Layout::for_value(&*ptr));
                }
            }
        }
    }
}
```

## `Weak<T>`

Rust在和`Rc<T>`的同时，提供了`Weak<T>`，`Weak<T>`和C++的`std::weak_ptr`功能相同。`Weak<T>`指示临时所有权，其所指向的对象可能在别处被释放。

Weak对象可以通过对Rc调用downgrade而得到:

```rust
// a is a 
let a : Rc<i32> = Rc::new(4);
let b : Weak<i32> = Rc::downgrade(&a);
```

Weak本身不能直接使用指向元素的值，必须将其转换成`Rc<T>`才可以使用。但由于Weak指向的元素可能已经被释放，因此实际上使用`Option<Rc<T>>`来指示转换的结果。

```rust
let b : Weak<i32> = Weak::new();
let c = Weak::upgrade(&b);
match c {
    Some(x) => println!("B is {}", x),
    None => println!("B is empty"),
}
```

对每一个`Option<T>`有两种可能：一种是`Some(T)`，表示存在，并且有一个值；另一种是`None`，表示没有值。

这对应着试图将`Weak<T>`转换成`Rc<T>`的两种结果：一种是指向的值还有效，得到`Some(Rc<T>)`，另一种情况是已经被释放，得到的结果就是`None`。具体实现如下：

```rust
    #[stable(feature = "rc_weak", since = "1.4.0")]
    pub fn upgrade(&self) -> Option<Rc<T>> {
        if self.strong() == 0 {
            None
        } else {
            self.inc_strong();
            Some(Rc { ptr: self.ptr })
        }
    }
```

为了实现这个功能，`RcBox<T>`会在释放完指向内存空间之后仍然维护指向它的`Weak`指针数，当该计数值也降为0后才释放堆上分配的空间。

## `Arc<T>`

Rust还提供了一种支持多线程使用的Reference Counted，`Arc<T>`，表示"Atomic Reference Counted"。其主要原理是使用原子操作来加减引用计数器，但其代价是程序的效率变低，因为每次修改操作都需要互斥。

其使用和实现与`Rc<T>`基本相同，同样使用`strong`和`weak`来指示指针数，但改为原子类型。

```rust
struct ArcInner<T: ?Sized> {
    strong: atomic::AtomicUsize,

    // the value usize::MAX acts as a sentinel for temporarily "locking" the
    // ability to upgrade weak pointers or downgrade strong ones; this is used
    // to avoid races in `make_mut` and `get_mut`.
    weak: atomic::AtomicUsize,

    data: T,
}

#[stable(feature = "rust1", since = "1.0.0")]
pub struct Arc<T: ?Sized> {
    ptr: Shared<ArcInner<T>>,
}
```

```rust
#[stable(feature = "rust1", since = "1.0.0")]
unsafe impl<#[may_dangle] T: ?Sized> Drop for Arc<T> {
    #[inline]
    fn drop(&mut self) {
        // Because `fetch_sub` is already atomic, we do not need to synchronize
        // with other threads unless we are going to delete the object. This
        // same logic applies to the below `fetch_sub` to the `weak` count.
        if self.inner().strong.fetch_sub(1, Release) != 1 {
            return;
        }

        // make sure all references are dropped before dropping the data
        atomic::fence(Acquire);

        unsafe {
            self.drop_slow();
        }
    }
}

#[inline(never)]
unsafe fn drop_slow(&mut self) {
    let ptr = self.ptr.as_ptr();

    // Destroy the data at this time, even though we may not free the box
    // allocation itself (there may still be weak pointers lying around).
    ptr::drop_in_place(&mut self.ptr.as_mut().data);

    if self.inner().weak.fetch_sub(1, Release) == 1 {
        atomic::fence(Acquire);
        Heap.dealloc(ptr as *mut u8, Layout::for_value(&*ptr))
    }
}
```

Rust 使用 atomic fence 机制来确保不会使用已经释放的数据。