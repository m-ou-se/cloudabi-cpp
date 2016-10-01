#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <type_traits>

namespace cloudabi {

template<typename T>
struct range {

	range() : _begin(nullptr), _size(0) {}

	range(T * b, size_t s) : _begin(b), _size(s) {}

	range(T * b, T * e) : _begin(b), _size(e - b) {}

	range(T & x) : _begin(&x), _size(1) {}

	template<std::size_t n> range(T (&x)[n]) : _begin(&x[0]), _size(n) {}

	template<std::size_t n> range(std::array<T, n> & x)
		: _begin(x.empty() ? nullptr : &*x.begin()), _size(x.size()) {}

	template<std::size_t n> range(std::array<std::remove_const_t<T>, n> const & x)
		: _begin(x.empty() ? nullptr : &*x.begin()), _size(x.size()) {}

	range(std::vector<T> & x) : _begin(&*x.begin()), _size(x.size()) {}

	range(std::vector<std::remove_const_t<T>> const & x) : _begin(&*x.begin()), _size(x.size()) {}

	template<typename T2> range(range<T2> const & r) : _begin(r.begin()), _size(r.size()) {}

	T & operator [] (std::size_t i) const { return _begin[i]; }

	T *   begin() const { return _begin; }
	T *     end() const { return _begin + _size; }
	T * & begin()       { return _begin; }
	T * &   end()       { return _begin + _size; }

	std::size_t size() volatile const { return _size; }

	T * data() const { return _begin; }

	bool empty() volatile const { return _size == 0; }

	T * _begin;
	size_t _size;
};

}
