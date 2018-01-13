# `Weak<T>`

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


