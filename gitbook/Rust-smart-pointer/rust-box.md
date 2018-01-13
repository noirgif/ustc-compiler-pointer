# `Box<T>`

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



