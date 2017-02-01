
#pragma once

#include <array>
#include <cstdlib>
#include <utility>

namespace arref {

template <typename T, class... I>
constexpr auto make_arref(T* d, I... ss);

template <size_t D>
struct dim {
    std::ptrdiff_t s;
};

namespace detail {

template <class>
struct is_ref_wrapper : std::false_type {};

template <class T>
struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

template <class B>
struct negation : std::integral_constant<bool, !B::value> {};

template <class...>
struct conjunction : std::true_type {};

template <class B1>
struct conjunction<B1> : B1 {};

template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<B1::value != false, conjunction<Bn...>, B1> {};

template <class... B>
constexpr bool conjunction_v = conjunction<B...>::value;

template <class T>
using not_ref_wrapper = negation<is_ref_wrapper<std::decay_t<T>>>;

template <class D, class...>
struct return_type_helper {
    using type = D;
};

template <class... Types>
struct return_type_helper<void, Types...> : std::common_type<Types...> {
    static_assert(conjunction_v<not_ref_wrapper<Types>...>,
                  "Types cannot contain reference_wrappers when D is void");
};

template <class D, class... Types>
using return_type = std::array<typename return_type_helper<D, Types...>::type,
                               sizeof...(Types)>;


template <typename Derived, typename T, size_t D>
struct arref_impl {
    arref_impl(const arref_impl&) noexcept = default;
    arref_impl(T* base) : base_{base} {}

    template <size_t DD>
    Derived& operator+=(const dim<DD>& d) {
        Derived& self = static_cast<Derived&>(*this);
        self.base_ += self.stride[D - DD - 1] * d.s;
        return self;
    }
    template <size_t DD>
    Derived& operator-=(const dim<DD>& d) {
        Derived& self = static_cast<Derived&>(*this);
        self.base_ -= self.stride[D - DD - 1] * d.s;
        return self;
    }

    template <typename... Is>
    auto transpose(Is... is) const {
        static_assert(sizeof...(Is) == D, "all dimensions must be assigned");
        return make_arref(
            base_, static_cast<const Derived*>(this)->stride[D - is - 1]...);
    }

    T* base_;
};

template <typename T, size_t S, size_t... Is>
std::array<T, sizeof...(Is)> tail_impl(const std::array<T, S>& x,
                                       std::index_sequence<Is...>) {
    return {{x[Is + 1]...}};
}

template <typename T, size_t S,
          typename Indices = std::make_index_sequence<S - 1>>
std::array<T, S - 1> tail(const std::array<T, S>& x) {
    return tail_impl(x, Indices());
}

}

template <class D = void, class... Types>
constexpr detail::return_type<D, Types...> make_array(Types&&... t) {
    return {{std::forward<Types>(t)...}};
}

template <typename T, size_t D>
struct arref : detail::arref_impl<arref<T, D>, T, D> {
    const std::array<size_t, D> stride;
    using impl = detail::arref_impl<arref<T, D>, T, D>;
    arref(T* base, const std::array<size_t, D>& s) : impl{base}, stride{s} {}

    arref& operator=(const arref& o) noexcept {
        impl::base_ = o.base_;
        return *this;
    }
    using impl::operator+=;
    using impl::operator-=;

    arref<T, D - 1> operator[](std::ptrdiff_t idx) {
        T* base = impl::base_ + stride[0] * idx;
        return {base, detail::tail(stride)};
    }
    const arref<T, D - 1> operator[](std::ptrdiff_t idx) const {
        T* base = impl::base_ + stride[0] * idx;
        return {base, detail::tail(stride)};
    }
};

template <typename T>
struct arref<T, 1> : detail::arref_impl<arref<T, 1>, T, 1> {
    using impl = detail::arref_impl<arref<T, 1>, T, 1>;
    const std::array<size_t, 1> stride;
    arref(T* base, const std::array<size_t, 1>& s) : impl{base}, stride{s} {}
    arref(T* base, std::array<size_t, 1>&& s) : impl{base}, stride{s} {}

    arref& operator=(const arref& o) noexcept {
        impl::base_ = o.base_;
    }
    using impl::operator+=;
    using impl::operator-=;

    T& operator[](std::ptrdiff_t idx) {
        return *(impl::base_ + stride[0] * idx);
    }
    const T& operator[](std::ptrdiff_t idx) const {
        return *(impl::base_ + stride[0] * idx);
    }
};

template <typename T, size_t D, size_t DN>
arref<T, D> operator+(const arref<T, D>& x, const dim<DN>& d) {
    arref<T, D> ret = x;
    return ret += d;
}

template <typename T, size_t D, size_t DN>
arref<T, D> operator-(const arref<T, D>& x, const dim<DN>& d) {
    arref<T, D> ret = x;
    return ret -= d;
}

template <typename T, class... I>
constexpr auto make_arref(T* d, I... ss) {
    return arref<T, sizeof...(ss)>(d, make_array<size_t>(ss...));
}
}
