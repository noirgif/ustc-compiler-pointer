# C++ raw pointer在应用中的缺陷和可能产生的问题

## 引言

C++语言与Java语言的不同之处，其中有一个一定要提到的就是指针了，Java是不存在指
针的，而C++的指针操作反而是C++语言的一个特点(并且因为没有直接对内存的操作，Java
又被称作是安全的编程语言)。为了探究C++指针指针的新特性，我们有必要先介绍一下
一般的C++指针以及其可能引发的问题。

## 指针的定义

指针通常意义上指的是存储了变量的内存地址，这是它和C一致的定义的。下面
是cplusplus.com上对内存地址的解释：
> In earlier chapters, variables have been explained as locations in the computer's memory which can be accessed by their identifier (their name). This way, the program does not need to care about the physical address of the data in memory; it simply uses the identifier whenever it needs to refer to the variable.
> For a C++ program, the memory of a computer is like a succession of memory cells, each one byte in size, and each with a unique address. These single-byte memory cells are ordered in a way that allows data representations larger than one byte to occupy memory cells that have consecutive addresses.
> This way, each cell can be easily located in the memory by means of its unique address. For example, the memory cell with the address 1776 always follows immediately after the cell with address 1775 and precedes the one with 1777, and is exactly one thousand cells after 776 and exactly one thousand cells before 2776.
> When a variable is declared, the memory needed to store its value is assigned a specific location in memory (its memory address). Generally, C++ programs do not actively decide the exact memory addresses where its variables are stored. Fortunately, that task is left to the environment where the program is run - generally, an operating system that decides the particular memory locations on runtime. However, it may be useful for a program to be able to obtain the address of a variable during runtime in order to access data cells that are at a certain position relative to it.

## 对指针的操作

取一个指针的地址 ：&
得到地址所存放的值 ：*
定义一个指针：type * name
指针的运算符： ++,--,[],=等

## 一般指针会带来的bug

- 悬空指针：

    悬空指针是指针算是最常见的一种bug了，而且编译器难以察觉。你声明了一个指
针，并且让它指向某个位置；但是由于种种原因，这个指针指向的内存空间要么已经被释
放了，要么这个指针所指向的位置根本是一个未定义的位置，这样就会引起悬空指针。悬
空指针就像定时炸弹，它给你的程序留下了隐患，并且你不知道什么时候它会突然显现出
来给你带来麻烦。
下面是两种常见的悬空指针的情况：
    (1)

>     int *a=new int(3);
>     //do something
>     delete a;
>     cout<<a;

    (引用已经被释放的空间)

    (2)

>     int *a=new int[3];
>     //do something
>     cout<<a[3];
>     delete a[];

    (引用没有被定义的空间)

- 内存泄漏：

    当你为一个指针分配内存的时候，那么直到程序运行结束或者这块内存被释放，这
块内存都是不能被其它程序所使用的。当然一个有着良好编程习惯的程序员自然会即时回
收这些已近用完但是依然还被占用的内存，但是如果你不去回收它，那就会造成内存泄
漏。尽管现在电脑的内存早已经上G，但是内存泄漏依然使你的可用内存减少，拖慢代码
运行速度。

- 不恰当的引用：

    这种情况尤其指在发生类型转换的时候，Atype继承了Btype，而当一个Atype的指针
试图访问Btype占有的内存的时候，编译器也许并不会报错，但是当实际执行的时候就会发
生 segmentation fault。

- 其它问题：

参考链接：https://isocpp.org
          http://www.cplusplus.com
          http://www.stackoverflow.com
参考文献：
          C Traps and Pitfalls
