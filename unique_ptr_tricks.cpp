// unique_ptr_tricks.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <iostream>

void deleter(int * p)
{
    std::cout << "deleter" << std::endl;
    delete p;
}

template <typename F, F f>
struct FBuilder;

template <typename... Args, void(f)(Args...)>
struct FBuilder<void (Args...), f>
{
    void operator()(Args&&... args)
    {
        return f(std::forward<Args>(args)...);
    }
};

static_assert(sizeof(FBuilder<decltype(deleter), deleter>) == 1, "bad size");

template< class T, class D, D d, class... Args >
auto make_unique_with_deleter(Args&&... args)
{
    return std::unique_ptr<int, FBuilder<D, d>>{ new T(std::forward<Args>(args)...) };
}

int main()
{

    //std::unique_ptr<int, void(&)(int*)> p{ new int(), deleter };
    auto p = make_unique_with_deleter<int, decltype(deleter), deleter>();
    static_assert(sizeof(p) == sizeof(int *), "bad size");
    return 0;
}

