#pragma once

#include <type_traits>

#include <mstd/range.hpp>
#include <mstd/string_view.hpp>

#include <cloudabi_types.h>

namespace cloudabi {

using mstd::range;
using mstd::string_view;

struct iovec : range<unsigned char> {
	using range::range;

	constexpr iovec(char * b, std::size_t s)
		: range((unsigned char *)b, s) {}

	constexpr iovec(char * b, char * e)
		: range((unsigned char *)b, (unsigned char *)e) {}

	iovec(char & x) : range((unsigned char &)x) {}

	template<std::size_t n>
		constexpr iovec(char (&x)[n]) : iovec(&x[0], n) {}

	template<
		typename X,
		typename = std::enable_if_t<
			(std::is_convertible<decltype(std::declval<X>().data()), char *>::value ||
			std::is_convertible<decltype(std::declval<X>().data()), unsigned char *>::value ) &&
			std::is_convertible<decltype(std::declval<X>().size()), std::size_t>::value
		>
	>
	constexpr iovec(X & x) : iovec(x.data(), x.size()) {}
};

static_assert(sizeof(cloudabi_iovec_t) == sizeof(iovec), "");
static_assert(alignof(cloudabi_iovec_t) == alignof(iovec), "");

struct ciovec : range<unsigned char const> {
	using range::range;

	constexpr ciovec(char const * b, std::size_t s)
		: range((unsigned char const *)b, s) {}

	constexpr ciovec(char const * b, char const * e)
		: range((unsigned char const *)b, (unsigned char const *)e) {}

	ciovec(char const & x) : ciovec(&x, 1) {}

	template<
		typename X,
		typename = std::enable_if_t<
			(std::is_convertible<decltype(std::declval<X>().data()), char const *>::value ||
			std::is_convertible<decltype(std::declval<X>().data()), unsigned char const *>::value ) &&
			std::is_convertible<decltype(std::declval<X>().size()), std::size_t>::value
		>
	>
	constexpr ciovec(X && x) : ciovec(x.data(), x.size()) {}

	constexpr ciovec(char const * s) : ciovec(string_view(s)) {}
};

static_assert(sizeof(cloudabi_ciovec_t) == sizeof(ciovec), "");
static_assert(alignof(cloudabi_ciovec_t) == alignof(ciovec), "");

}
