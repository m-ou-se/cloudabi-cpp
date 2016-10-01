#pragma once

#include <cassert>
#include <new>
#include <utility>

#include "cloudabi_types.hpp"

namespace cloudabi {

template<typename T>
class error_or {

private:

	cloudabi::error error_ = cloudabi::error::inval;

	union { T value_; };

public:

	// Implicit conversions from both cloudabi::error and T.

	error_or(cloudabi::error e) : error_(e) {
		// An error_or<T> without an error needs a value.
		assert(!ok());
	}

	error_or(T value) {
		new (&value_) T(std::move(value));
		// Only set error to 'no_error' afterwards, to prevent
		// destructing value_ when the constructor throws an error.
		error_ = cloudabi::error(0);
	}

	// Move and copy constructors.

	error_or(error_or && other) noexcept(
		noexcept(T(std::move(other.value_)))
	) {
		if (other.ok()) new (&value_) T(std::move(other.value_));
		error_ = other.error_;
	}

	error_or(error_or const & other) {
		if (other.ok()) new (&value_) T(other.value_);
		error_ = other.error_;
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
		error_ = other.error_;
	}

	error_or & operator = (error_or const & other) {
		if (ok() && other.ok()) {
			value_ = other.value_;
		} else if (ok()) {
			value_.~T();
		} else if (other.ok()) {
			new (&value_) T(other.value_);
		}
		error_ = other.error_;
	}

	// Destructor.

	~error_or() noexcept(noexcept(value_.~T())) {
		if (ok()) value_.~T();
	}

	// Explicit accessors.

	bool ok() const { return error_ != cloudabi::error(0); }

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

	// Comparison between error_or and error.

	friend inline bool operator == (error_or a, enum error b) {
		return a.error_ == b;
	}

	friend inline bool operator != (error_or a, enum error b) {
		return a.error_ != b;
	}

	friend inline bool operator == (enum error a, error_or b) {
		return a == b.error_;
	}

	friend inline bool operator != (enum error a, error_or b) {
		return a != b.error_;
	}

	// Comparison between error_or and T.

	friend inline bool operator == (error_or a, T const & b) {
		if (a.error_) return false;
		return a.value_ == b;
	}

	friend inline bool operator != (error_or a, T const & b) {
		if (a.error_) return true;
		return a.value_ != b;
	}

	friend inline bool operator == (T const & a, error_or b) {
		if (b.error_) return false;
		return a == b.value_;
	}

	friend inline bool operator != (T const & a, error_or b) {
		if (b.error_) return true;
		return a != b.value_;
	}

	// Comparison between error_ors.

	template<typename T1, typename T2> friend inline bool operator == (error_or<T1> a, error_or<T2> b) {
		if (a.error_ != b.error_) return false;
		if (a.error_) return true;
		return a.value_ == b.value_;
	}

	template<typename T1, typename T2> friend inline bool operator != (error_or<T1> a, error_or<T2> b) {
		if (a.error_ != b.error_) return true;
		if (a.error_) return false;
		return a.value_ != b.value_;
	}

};

template<>
class error_or<void> {

private:

	cloudabi::error error_ = cloudabi::error::inval;

public:

	// Implicit conversions from both cloudabi::error and T.

	error_or(cloudabi::error e) : error_(e) {}

	error_or() : error_(cloudabi::error(0)) {}

	// Accessors.

	bool ok() const { return error_ != cloudabi::error(0); }

	explicit operator bool() const { return ok(); }

	cloudabi::error error() const { return error_; }

	// Comparison between error_or and error.

	friend inline bool operator == (error_or a, enum error b) {
		return a.error_ == b;
	}

	friend inline bool operator != (error_or a, enum error b) {
		return a.error_ != b;
	}

	friend inline bool operator == (enum error a, error_or b) {
		return a == b.error_;
	}

	friend inline bool operator != (enum error a, error_or b) {
		return a != b.error_;
	}

	// Comparison between error_ors.

	friend inline bool operator == (error_or a, error_or b) {
		return a.error_ == b.error_;
	}

	friend inline bool operator != (error_or a, error_or b) {
		return a.error_ != b.error_;
	}

};

}
