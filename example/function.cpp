#include "currying.hpp"
#include<iostream>
using namespace std;
int foo(int a, int b, int c, int d)
{
    cout <<"in foo    "<< a << "  " << b << "  " << c << "  " << d << endl;
    return a + b + c + d; 
}
int main()
{
    using namespace FPinCpp;
    auto a = Currying(foo);
    cout <<"in main  "<< a(1, 2, 3, 4) << endl;
    auto b = a(_, _, 1);
    auto c = b(2, 3);
    auto d = c(4);
    return 0;
}
//output:
//in foo   1  2  3  4
//in main  10
//in foo   2  3  1  4

