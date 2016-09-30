#pragma once

#include <cloudabi_types.h>

#include "range.hpp"

namespace cloudabi {

using iovec = range<unsigned char>;

static_assert(sizeof(cloudabi_iovec_t) == sizeof(iovec), "");
static_assert(alignof(cloudabi_iovec_t) == alignof(iovec), "");
static_assert(sizeof(cloudabi_iovec_t::iov_base) == sizeof(iovec::_begin), "");
static_assert(sizeof(cloudabi_iovec_t::iov_len) == sizeof(iovec::_size), "");
static_assert(offsetof(cloudabi_iovec_t, iov_base) == offsetof(iovec, _begin), "");
static_assert(offsetof(cloudabi_iovec_t, iov_len) == offsetof(iovec, _size), "");

using ciovec = range<unsigned char const>;

static_assert(sizeof(cloudabi_ciovec_t) == sizeof(ciovec), "");
static_assert(alignof(cloudabi_ciovec_t) == alignof(ciovec), "");
static_assert(sizeof(cloudabi_ciovec_t::iov_base) == sizeof(ciovec::_begin), "");
static_assert(sizeof(cloudabi_ciovec_t::iov_len) == sizeof(ciovec::_size), "");
static_assert(offsetof(cloudabi_ciovec_t, iov_base) == offsetof(ciovec, _begin), "");
static_assert(offsetof(cloudabi_ciovec_t, iov_len) == offsetof(ciovec, _size), "");

}
