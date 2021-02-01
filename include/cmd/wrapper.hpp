#pragma once

#include <com/fatal_error.hpp>
#include <com/variant.hpp>
#include <tuple>

namespace miu::cmd {

template<typename tuple_type, uint32_t I>
struct collector {
    auto operator()(tuple_type& collected, com::variant const* args) {
        auto constexpr IDX = I - 1;
        using ARG          = typename std::tuple_element<IDX, tuple_type>::type;

        std::get<IDX>(collected) = args[IDX].get<ARG>().value();
        return collector<tuple_type, IDX>()(collected, args);
    }
};

template<typename tuple_type>
struct collector<tuple_type, 0> {
    auto operator()(tuple_type& collected, com::variant const*) { return collected; }
};

/////////////////////////////////////////////////////
template<typename... ARGS>
auto collect(com::variant const* vars, uint32_t size) {
    using tuple_type          = std::tuple<ARGS...>;
    auto constexpr tuple_size = std::tuple_size<tuple_type>::value;

    if (tuple_size > size) {
        FATAL_ERROR<std::logic_error>("lack of args", size, "<", tuple_size);
    }

    tuple_type collected;
    return collector<tuple_type, tuple_size>()(collected, vars);
}

/////////////////////////////////////////////////////
template<typename>
struct wrapper;

template<typename T, typename... ARGS>
struct wrapper<void (T::*)(ARGS...)> {
    static com::variant
    bounce(void (T::*f)(ARGS...), T* t, com::variant const* vars, uint32_t size) {
        auto args = collect<ARGS...>(vars, size);
        std::apply([&](auto... args) { return (t->*f)(args...); }, args);
        return {};
    }
};

template<typename R, typename T, typename... ARGS>
struct wrapper<R (T::*)(ARGS...)> {
    static com::variant bounce(R (T::*f)(ARGS...), T* t, com::variant const* vars, uint32_t size) {
        auto args = collect<ARGS...>(vars, size);
        return { std::apply([&](auto... args) { return (t->*f)(args...); }, args) };
    }
};

template<typename... ARGS>
struct wrapper<void (*)(ARGS...)> {
    static com::variant bounce(void (*f)(ARGS...), com::variant const* vars, uint32_t size) {
        auto args = collect<ARGS...>(vars, size);
        std::apply(f, args);
        return {};
    }
};

template<typename R, typename... ARGS>
struct wrapper<R (*)(ARGS...)> {
    static com::variant bounce(R (*f)(ARGS...), com::variant const* vars, uint32_t size) {
        auto args = collect<ARGS...>(vars, size);
        return { std::apply(f, args) };
    }
};

}    // namespace miu::cmd
