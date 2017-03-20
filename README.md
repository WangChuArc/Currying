## C++中的柯里化      

**轻量化:**只有一个hpp文件。

**使用简单：**只需一步即可完成函数柯里化。

**要求:**支持c++14的编译器。确切地说，我是用的Visual Studio 2015 update2。暂未在GCC上测试。

**说明：**
```c++
auto curried = currying(func);
```
`curried`即为柯里化后函数。严格地说不能算柯里化，因为你一次输入多个参数也是可以的。可使用"_"作为占位符。暂只支持柯里化后函数生成在栈中。任何意见建议请致信hoho71888675@126.com,感激不尽。


**update 17-3-20** 使用过程中发现了一些问题。主要是VS和gcc编译器关于calling convention的指定方式不一致。VS编译器没有支持C++11标准里的`attribute`属性部分。另外Qt项目也不支持直接指定VS式的直接指定calling convention，哪怕用MSVC编译器来编译。因此这个项目现在只能用VS2015来编译。


**update 16-12-31**：16年最后一天的更新。。已经修复柯里化后的函数不能保存原来calling convention的问题了。现支持'cdecl、stdcall、fastcall、vectorcall'4种calling convention。

TODO:
增加延迟调用功能。e.g.:int foo(int, int, int);  auto f = currying(foo);  auto g = f(1, 2, 3);  int i = g();  形如上述。当然延迟调用功能会做成可选的，可以开启关闭的。

TODO：
柯里化后的函数不能给std::function赋值，需要重载'='运算符。
