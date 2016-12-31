# C++中的柯里化      

轻量化:只有一个hpp文件。

使用简单：只需一步即可完成函数柯里化。

要求:支持c++14的编译器。确切地说，我是用的Visual Studio 2015 update2。暂未在GCC上测试。

说明：auto curried = currying(func);curried即为柯里化后函数。严格地说不能算柯里化，因为你一次输入多个参数也是可以的。可使用"_"作为占位符。暂只支持柯里化后函数生成在栈中。任何意见建议请致信hoho71888675@126.com,感激不尽。

16-12-31更新：16年最后一天的更新。。已经修复柯里化后的函数不能保存原来calling convention的问题了。现支持'cdecl、stdcall、fastcall、vectorcall'4种calling convention。
