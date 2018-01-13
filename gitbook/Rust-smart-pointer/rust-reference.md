# 引用

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

## 对竞态的保护

Rust中，一个值最多同时只能够有一个`&mut T`引用（可更改值的引用）或若干个`&T`引用（只读的引用），这种限制避免了在一处使用引用值前被另一处的程序更改的情况。

## 对悬垂引用的保护

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



