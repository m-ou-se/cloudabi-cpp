#pragma once

#include <mstd/range.hpp>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error_or.hpp"
#include "fd.hpp"
#include "types.hpp"

namespace cloudabi {

using mstd::range;

inline error_or<void> mem_advise(range<unsigned char> mem, advice a) {
	return error(cloudabi_sys_mem_advise(mem.data(), mem.size(), cloudabi_advice_t(a)));
}

inline error_or<void> mem_lock(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_lock(mem.data(), mem.size()));
}

inline error_or<void *> mem_map(
	size_t len,
	mprot prot = mprot::read | mprot::write,
	mflags flags = mflags::private_,
	void * addr = nullptr
) {
	return fd(CLOUDABI_MAP_ANON).mem_map(len, 0, prot, flags, addr);
}

inline error_or<void> mem_protect(range<unsigned char> mem, mprot prot) {
	return error(cloudabi_sys_mem_protect(mem.data(), mem.size(), cloudabi_mprot_t(prot)));
}

inline error_or<void> mem_sync(range<unsigned char> mem, msflags flags) {
	return error(cloudabi_sys_mem_sync(mem.data(), mem.size(), cloudabi_msflags_t(flags)));
}

inline error_or<void> mem_unlock(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_unlock(mem.data(), mem.size()));
}

inline error_or<void> mem_unmap(range<unsigned char> mem) {
	return error(cloudabi_sys_mem_unmap(mem.data(), mem.size()));
}

}
