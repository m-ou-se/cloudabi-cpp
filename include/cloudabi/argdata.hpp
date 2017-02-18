#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include <argdata.h>

#include <mstd/optional.hpp>
#include <mstd/range.hpp>
#include <mstd/string_view.hpp>

namespace cloudabi {

using mstd::optional;
using mstd::range;
using mstd::string_view;

class fd;

class argdata_map;
class argdata_seq;

// XXX: UNTESTED DRAFT
class argdata {

public:
	argdata() = delete;
	argdata(argdata const &) = delete;
	argdata &operator=(argdata const &) = delete;

	void operator delete (void *p) {
		argdata_free(static_cast<argdata *>(p));
	}

	static std::unique_ptr<argdata> create_encoded(mstd::range<unsigned char> r) {
		return std::unique_ptr<argdata>(argdata_create_buffer(r.data(), r.size()));
	}
	static std::unique_ptr<argdata> create_binary(mstd::range<unsigned char> r) {
		return std::unique_ptr<argdata>(argdata_create_binary(r.data(), r.size()));
	}
	static std::unique_ptr<argdata> create_fd(int v) {
		return std::unique_ptr<argdata>(argdata_create_fd(v));
	}
	static std::unique_ptr<argdata> create_float(double v) {
		return std::unique_ptr<argdata>(argdata_create_float(v));
	}
	static std::unique_ptr<argdata> create_int(std::uintmax_t v) {
		return std::unique_ptr<argdata>(argdata_create_int(v));
	}
	static std::unique_ptr<argdata> create_int(std::intmax_t v) {
		return std::unique_ptr<argdata>(argdata_create_int(v));
	}
	static std::unique_ptr<argdata> create_int(int v) {
		return std::unique_ptr<argdata>(argdata_create_int(v));
	}
	static std::unique_ptr<argdata> create_map(range<argdata const *const> keys, range<argdata const *const> values) {
		return std::unique_ptr<argdata>(argdata_create_map(keys.data(), values.data(), keys.size() < values.size() ? keys.size() : values.size());
	}
	static std::unique_ptr<argdata> create_seq(range<argdata const *const> values) {
		return std::unique_ptr<argdata>(argdata_create_seq(values.data(), values.size());
	}
	static std::unique_ptr<argdata> create_str(string_view v) {
		return std::unique_ptr<argdata>(argdata_create_str(v.data(), v.size());
	}
	//TODO: static argdata create_timestamp(timespec const *);
	
	static constexpr argdata *false_() { return &argdata_false; }
	static constexpr argdata *true_ () { return &argdata_true ; }
	static constexpr argdata *null  () { return &argdata_null ; }

	static constexpr argdata *bool_(bool v) { return v ? true_() : false_(); }

	optional<range<unsigned char>> get_binary() const {
		void const *data;
		size_t size;
		if (argdata_get_binary(this, &data, &size)) return {};
		return range<unsigned char>{static_cast<unsigned char *>(data), size};
	}
	optional<bool> get_bool() const {
		bool r;
		if (argdata_get_bool(this, &r)) return {};
		return r;
	}
	optional<int> get_fd() const {
		int r;
		if (argdata_get_fd(this, &r)) return {};
		return r;
	}
	optional<double> get_float() const {
		double r;
		if (argdata_get_float(this, &r)) return {};
		return r;
	}
	optional<std::intmax_t> get_int() const {
		std::intmax_t r;
		if (argdata_get_int(this, &r)) return {};
		return r;
	}
	optional<std::uintmax_t> get_uint() const {
		std::uintmax_t r;
		if (argdata_get_int(this, &r)) return {};
		return r;
	}
	optional<string_view> get_str() const {
		char const *data;
		size_t size;
		if (argdata_get_str(this, &data, &size)) return {};
		return string_view(data, size);
	}
	//TODO: optional<timespec> get_timestamp() const;

	// Same as above, but return a default value (empty/zero/etc.) instead of nullopt.
	range<unsigned char> as_binary() const { return get_binary().value_or(range<unsigned char>{}); }
	bool                 as_bool  () const { return get_bool  ().value_or(                 false); }
	int                  as_fd    () const { return get_fd    ().value_or(                    -1); }
	double               as_float () const { return get_float ().value_or(                   0.0); }
	std::intmax_t        as_int   () const { return get_int   ().value_or(                     0); }
	std::uintmax_t       as_uint  () const { return get_uint  ().value_or(                     0); }
	string_view          as_str   () const { return get_str   ().value_or(         string_view{}); }
	//TODO: timespec as_timestamp() const { return get_timestamp().value_or({}); }

	class map {
	private:
		argdata_map_iterator it_;
		friend class argdata;
	public:
		optional<std::pair<argdata *, argdata *>> next() {
			std::pair<argdata *, argdata *> r;
			if (argdata_map_next(&it_, &r.first, &r.second)) return r;
			return {};
		}
	};
	class seq {
	private:
		argdata_seq_iterator it_;
		friend class argdata;
	public:
		optional<argdata *> next() {
			argdata *r;
			if (argdata_seq_next(&it_, &r)) return r;
			return {};
		}
	};

	optional<argdata_map> get_map() const {
		map r;
		if (argdata_map_iterate(this, &r.it_)) return {};
		return r;
	}
	optional<argdata_seq> get_seq() const {
		seq r;
		if (argdata_seq_iterate(this, &r.it_)) return {};
		return r;
	}

	optional<argdata_map> as_map() const {
		map r;
		argdata_map_iterate(this, &r.it_);
		return r;
	}
	optional<argdata_seq> as_seq() const {
		seq r;
		argdata_seq_iterate(this, &r.it_);
		return r;
	}

	size_t encoded_size(size_t *n_fds = nullptr) const {
		size_t r;
		argdata_get_buffer_length(this, &r, n_fds);
		return r;
	}

	void encode(std::vector<unsigned char> &buffer) const {
		buffer.resize(encoded_size());
		argdata_get_buffer(this, buffer.data(), nullptr);
	}

	void encode(std::vector<unsigned char> &buffer, std::vector<int> &fds) const {
		size_t n_fds;
		buffer.resize(encoded_size(&n_fds));
		fds.resize(n_fds);
		argdata_get_buffer(this, buffer.data(), fds.data());
		// TODO: Check argdata_get_buffer return value? Should it always equal n_fds?
	}

	std::vector<unsigned char> encode(std::vector<int> *fds = nullptr) const {
		std::vector<unsigned char> buffer;
		if (fds) encode(buffer, *fds);
		else encode(buffer);
		return buffer;
	}

	void print_yaml(FILE *f) {
		argdata_print_yaml(this, f);
	}

};

}
