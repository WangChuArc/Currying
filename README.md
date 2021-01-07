## C++中的柯里化      

** 轻量化: **只有一个hpp文件。

** 使用简单: **只需一步即可完成函数柯里化。
比如：
```
  	auto f = [](int a, float& b, char c){
		cout << a << endl;
		cout << b << endl;
		cout << c << endl;
		b += 10;

		return 1; 
	};

	int a = 15;
	float b = 10.f;
	char c = 'c';

	auto f1 = currying(f);

	auto f2 = f1(_, b); // "_"是通配符"Wildcard"的一个实例。用户也可以自己构造: Wildcard{}

	auto f3 = f2(35);

	auto r = f3(c);

	cout << b << endl;
  
  // 35 10 'c' 20
```

**更新更新！**

又更新辣。

为CurriedFunction增加了一个可选接口invoke，用于选择CurriedFuntion内保存的参数的传递方式。有eRef(默认), eCopy, eMove三种。
