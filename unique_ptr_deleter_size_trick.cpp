#include <memory>
#include <cassert>
#include <type_traits>
#include "FunctionFunctor.h"

void foo0() {}
void foo1(int) {}
void foo2(int, float) {}

int bar0() { return 0; }
float bar1(int) { return 0; }
double bar2(int, float) { return 0; }


static_assert(std::is_empty<FUNCTION_FUNCTOR(foo0)>::value, "FunctionFunctor not empty !");
static_assert(std::is_empty<FUNCTION_FUNCTOR(foo1)>::value, "FunctionFunctor not empty !");
static_assert(std::is_empty<FUNCTION_FUNCTOR(foo2)>::value, "FunctionFunctor not empty !");
static_assert(std::is_empty<FUNCTION_FUNCTOR(bar0)>::value, "FunctionFunctor not empty !");
static_assert(std::is_empty<FUNCTION_FUNCTOR(bar1)>::value, "FunctionFunctor not empty !");
static_assert(std::is_empty<FUNCTION_FUNCTOR(bar2)>::value, "FunctionFunctor not empty !");

// a make_unique with deleter with a trick to limit size
template< class T, class D, class... Args >
auto make_unique_with_deleter(Args&&... args)
{
    return std::unique_ptr<T, D>{ new T(std::forward<Args>(args)...) };
}

template< class T, void f(T*), class... Args >
auto make_unique_with_deleter(Args&&... args)
{
    return std::unique_ptr<T, FUNCTION_FUNCTOR(f)>{ new T(std::forward<Args>(args)...) };
}

// test

template <typename T>
bool g_deleterCalled = false;

template <typename T>
void deleter(T * p)
{
    g_deleterCalled<T> = true;
    delete p;
}
static_assert(std::is_empty<FUNCTION_FUNCTOR(deleter<int>)>::value, "FunctionFunctor not empty !");

int main()
{
    {
        // The common way
        std::unique_ptr<int, void(*)(int*)> p{ new int(), deleter<int> };
        static_assert(sizeof(p) == sizeof(int *) + sizeof(void(*)(int*)), "Not the expected size");
    }
    assert(g_deleterCalled<int>);

    {
        // templating with the function
        auto p = make_unique_with_deleter<float, deleter<float>>();
        static_assert(sizeof(p) == sizeof(float *), "No ebo in implementation or ebo failure");
    }
    assert(g_deleterCalled<float>);

    {
        // check functors are still ok
        auto p_functor = make_unique_with_deleter<int, std::default_delete<int>>();
        static_assert(sizeof(p_functor) == sizeof(int *), "No ebo in implementation or ebo failure");
    }

    return 0;
}

