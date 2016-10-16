#include <memory>
#include <cassert>
#include <type_traits>

// FunctionFunctor : A functor class calling the given function.
// Can be use as base for empty base optimization (ebo).

#if __cplusplus > 201402L
// c++17 : use auto for non type template parameter
// 

template <auto f>
struct FunctionFunctor
{
    template <typename Args>
    auto operator()(Args&&... args)
    {
        return f(std::forward<Args>(args)...);
    }
};

// Macro for before c++17
#define FUNCTION_FUNCTOR(f) FunctionFunctor<f>

#else
template <typename F, F f> // or auto F in c++17
struct FunctionFunctor;

template <typename... Args, typename R, R(*f) (Args...)>
struct FunctionFunctor<R(Args...), f>
{
    auto operator()(Args... args)
    {
        return f(std::forward<Args>(args)...);
    }
};

// second specialization if the function has been decayed...
template <typename... Args, typename R, R(*f) (Args...)>
struct FunctionFunctor<R(*)(Args...), f>
{
    auto operator()(Args... args)
    {
        return f(std::forward<Args>(args)...);
    }
};

// macro to avoid repeating f.
// Probably that C++17 auto non type template will improve it.
#define FUNCTION_FUNCTOR(f) FunctionFunctor<decltype(f), f>
#endif

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

