#pragma once

#include <cassert>
#include <new>
#include <utility>

#include "error.hpp"

namespace cloudabi {

template<typename T>
class error_or {

private:

	cloudabi::error error_ = cloudabi::error::inval;

	union { T value_; };

public:

	// Implicit conversions from both cloudabi::error and T.

	error_or(cloudabi::error e) : error_(e) {
		assert(error_ != cloudabi::error::no_error);
	}

	error_or(T value) {
		new (&value_) T(std::move(value));
		// Only set error to 'no_error' afterwards, to prevent
		// destructing value_ when the constructor throws an error.
		error_ = cloudabi::error::no_error;
	}

	// Move and copy constructors.

	error_or(error_or && other) noexcept(
		noexcept(T(std::move(other.value_)))
	) {
		if (other.ok()) new (&value_) T(std::move(other.value_));
		error_ = other.error;
	}

	error_or(error_or const & other) {
		if (other.ok()) new (&value_) T(other.value_);
		error_ = other.error;
	}

	// Move and copy assignment.

	error_or & operator = (error_or && other) noexcept(
		noexcept(value_ = std::move(other.value_)) &&
		noexcept(value_.~T()) &&
		noexcept(T(std::move(other.value_)))
	) {
		if (ok() && other.ok()) {
			value_ = std::move(other.value_);
		} else if (ok()) {
			value_.~T();
		} else if (other.ok()) {
			new (&value_) T(std::move(other.value_));
		}
		error_ = other.error;
	}

	error_or & operator = (error_or const & other) {
		if (ok() && other.ok()) {
			value_ = other.value_;
		} else if (ok()) {
			value_.~T();
		} else if (other.ok()) {
			new (&value_) T(other.value_);
		}
		error_ = other.error;
	}

	// Destructor.

	~error_or() noexcept(noexcept(value_.~T())) {
		if (ok()) value_.~T();
	}

	// Explicit accessors.

	bool ok() const { return error_ == cloudabi::error::no_error; }

	cloudabi::error error() const { return error_; }

	T       &  value()       &  { return value_; }
	T const &  value() const &  { return value_; }
	T       && value()       && { return std::move(value_); }

	// Implicit accessors.

	explicit operator bool() const { return ok(); }

	T       &  operator * ()       &  { return value_; }
	T const &  operator * () const &  { return value_; }
	T       && operator * ()       && { return std::move(value_); }

	T       * operator -> ()       { return value_; }
	T const * operator -> () const { return value_; }
};

// Comparison between error_or and error.

template<typename T> inline bool operator == (error_or<T> a, error b) {
	return a.error() == b;
}

template<typename T> inline bool operator != (error_or<T> a, error b) {
	return a.error() != b;
}

template<typename T> inline bool operator == (error a, error_or<T> b) {
	return a == b.error();
}

template<typename T> inline bool operator != (error a, error_or<T> b) {
	return a != b.error();
}

// Comparison between error_or and T.

template<typename T> inline bool operator == (error_or<T> a, T const & b) {
	if (!a.ok()) return false;
	return a.value() == b;
}

template<typename T> inline bool operator != (error_or<T> a, T const & b) {
	if (!a.ok()) return true;
	return a.value() != b;
}

template<typename T> inline bool operator == (T const & a, error_or<T> b) {
	if (!b.ok()) return false;
	return a == b.value();
}

template<typename T> inline bool operator != (T const & a, error_or<T> b) {
	if (!b.ok()) return true;
	return a != b.value();
}

// Comparison between error_ors.

template<typename T1, typename T2> inline bool operator == (error_or<T1> a, error_or<T2> b) {
	if (a.error() != b.error()) return false;
	if (!a.ok()) return true;
	return a.value() == b.value();
}

template<typename T1, typename T2> inline bool operator != (error_or<T1> a, error_or<T2> b) {
	if (a.error() != b.error()) return true;
	if (!a.ok()) return false;
	return a.value() != b.value();
}

}
