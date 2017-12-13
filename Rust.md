# Rust 中的智能指针

Rust在过去的版本中有managed pointer（用~和@表示，类似C++的`std::unique_ptr`和`std::shared_ptr`，在现在的版本中已经被移除，现在rust只有raw pointer和reference。

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

##`Box<T>`

如其名所提示的，`Box<T>`提供的是对堆上分配内存的最简单实现，创立Box对象时，会分配堆上的空间当Box对象销毁时，会将带有的object一并销毁。

`Box::new`会在堆上分配能够保存接受该数据的空间，并返回指向该空间的指针(`std::boxed::Box<T>`)。由于历史原因，Box不像一般的引用一样需要`*`解引用。

```rust
let x = Box::new(5);
print("x is {}", x);
```

会输出`x is 5` 。