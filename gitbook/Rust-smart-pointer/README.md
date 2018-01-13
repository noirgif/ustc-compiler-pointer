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




