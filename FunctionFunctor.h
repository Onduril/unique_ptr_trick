#ifndef FUNCTION_FUNCTOR_H_
#define FUNCTION_FUNCTOR_H_

// FunctionFunctor : A functor class calling the given function.
// Can be use as base for empty base optimization (ebo) in unique_ptr.

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

#endif // FUNCTION_FUNCTOR_H_
