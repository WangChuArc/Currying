#include "currying.hpp"
#include<iostream>
using namespace std;
struct foo
{
    int m;
    int operator()(int a, int b, int c)
    {
        cout << "in foo  " << a << "  " << b << "  " << c << endl;
        return a + b + c + m;
    }
};
int main()
{
    using namespace FPinCpp;
    auto obj_a = foo{ 10 };
    foo* ptr = &obj_a;
    auto a = Currying(obj_a); //pass by value
    auto b = a(1, 2);
    cout << "in main  " << b(3) << endl;
    ptr->m = 100;
    cout << "in main  " << b(3) << endl;

    cout << "--------------------------------------------------------" << endl;

    auto obj_b = foo{ 10 };
    ptr = &obj_b;
    auto c = Currying(&obj_b); // pass by pointer
    auto d = c(1, 2);
    cout << "in main  " << d(3) << endl;
    ptr->m = 100;
    cout << "in main  " << d(3) << endl;
    return 0;
}
//output:
//in foo   1  2  3
//in main  16
//in foo   1  2  3
//in main  16
//--------------------------------------------------------
//in foo   1  2  3
//in main  16
//in foo   1  2  3
//in main  106

