#pragma once

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error.hpp"
#include "range.hpp"
#include "types.hpp"

namespace cloudabi {

inline error mem_advise(range<unsigned char> mem, advice a) {
	return error(cloudabi_sys_mem_advise(mem.data(), mem.size(), cloudabi_advice_t(a)));
}

inline error mem_lock(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_lock(mem.data(), mem.size()));
}

inline error mem_protect(range<unsigned char> mem, mprot prot) {
	return error(cloudabi_sys_mem_protect(mem.data(), mem.size(), cloudabi_mprot_t(prot)));
}

inline error mem_sync(range<unsigned char> mem, msflags flags) {
	return error(cloudabi_sys_mem_sync(mem.data(), mem.size(), cloudabi_msflags_t(flags)));
}

inline error mem_unlock(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_unlock(mem.data(), mem.size()));
}

inline error mem_unmap(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_unmap(mem.data(), mem.size()));
}

}
