#include "currying.hpp"
#include<iostream>
using namespace std;
struct foo
{
    int m;
    int bar(int a, int b, int c, int d)
    {
        cout << "in bar  " << a << "  " << b << "  " << c << "  " << d << endl;
        return a + b + c + d + m;
    }
};
int main()
{
    using namespace FPinCpp;
    auto a = Currying(&foo::bar); //you can't omit '&' here, and you have to designate an object of 'foo' for 'bar'
    foo* ptr = nullptr;
    foo obj = foo{ 10 };
    ptr = &obj;
    auto b = a(ptr);
    auto c = b(_, _, 1);
    auto d = c(2, 3);
    cout << "in main  " << d(4) << endl;
    ptr->m = 100;
    cout << "in main  " << d(4) << endl;
    return 0;
}
//output:
//in bar   2  3  1  4
//in main  20
//in foo   2  3  1  4
//in main  110

