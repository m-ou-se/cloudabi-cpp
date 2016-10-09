#pragma once

#include <mstd/range.hpp>

#include <cloudabi_types.h>

namespace cloudabi {

using mstd::range;

using iovec = range<unsigned char>;

static_assert(sizeof(cloudabi_iovec_t) == sizeof(iovec), "");
static_assert(alignof(cloudabi_iovec_t) == alignof(iovec), "");

using ciovec = range<unsigned char const>;

static_assert(sizeof(cloudabi_ciovec_t) == sizeof(ciovec), "");
static_assert(alignof(cloudabi_ciovec_t) == alignof(ciovec), "");

}
