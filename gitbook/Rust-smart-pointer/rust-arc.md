# `Arc<T>`

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

