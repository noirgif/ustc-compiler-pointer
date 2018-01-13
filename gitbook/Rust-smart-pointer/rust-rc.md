# `Rc<T>`

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



