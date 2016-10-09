#pragma once

#include <mstd/range.hpp>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error_or.hpp"

namespace cloudabi {

using mstd::range;

inline error_or<void> random_get(range<unsigned char> buf) {
	return error(cloudabi_sys_random_get(buf.data(), buf.size()));
}

}
