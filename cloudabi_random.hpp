#pragma once

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "cloudabi_range.hpp"
#include "cloudabi_types.hpp"

namespace cloudabi {

inline error random_get(range<unsigned char> buf) {
	return error(cloudabi_sys_random_get(buf.data(), buf.size()));
}

}
