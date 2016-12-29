#include "currying.hpp"
#include<iostream>
using namespace std;

int main()
{
    using namespace FPinCpp;

    int i = 10, j = 10;
    auto fn = [&i, j](int a, int b) -> int 
    {
        cout << "in lambda  " << a << "  " << b << "  " << i << "  " << j << endl;
        return a + b + i + j; 
    };
    auto a = Currying(fn); 
    auto b = a(1);
    cout << "in main  " << b(2) << endl;
    i = 100, j = 100;
    cout << "in main  " << b(3) << endl;

    return 0;
}
//output:
//in lambda  1  2  10  10
//in main  23
//in lambda  1  3  100 10
//in main  114

