#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <type_traits>

namespace cloudabi {

template<typename T>
struct range {

	range() : begin_(nullptr), end_(nullptr) {}

	range(T * b, T * e) : begin_(b), end_(e) {}

	range(T & x) : begin_(&x), end_(&x+1) {}

	template<std::size_t n> range(T (&x)[n]) : begin_(&x[0]), end_(&x[n]) {}

	template<std::size_t n> range(std::array<T, n> & x)
		: begin_(x.empty() ? nullptr : &*x.begin()), end_(begin_ + x.size()) {}

	template<std::size_t n> range(std::array<std::remove_const_t<T>, n> const & x)
		: begin_(x.empty() ? nullptr : &*x.begin()), end_(begin_ + x.size()) {}

	range(std::vector<T> & x) : begin_(&*x.begin()), end_(begin_ + x.size()) {}

	range(std::vector<std::remove_const_t<T>> const & x) : begin_(&*x.begin()), end_(begin_ + x.size()) {}

	template<typename T2> range(range<T2> const & r) : begin_(r.begin()), end_(r.end()) {}

	T & operator [] (std::size_t i) const { return begin_[i]; }

	T *   begin() const { return begin_; }
	T *     end() const { return   end_; }
	T * & begin()       { return begin_; }
	T * &   end()       { return   end_; }

	std::size_t size() volatile const { return std::size_t(end_ - begin_); }

	T * data() const { return begin_; }

	bool empty() volatile const { return size() == 0; }

private:
	T * begin_;
	T *   end_;

};

}
