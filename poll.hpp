#pragma once

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error.hpp"
#include "error_or.hpp"
#include "range.hpp"
#include "types.hpp"

namespace cloudabi {

// TODO: Make cloudabi::subscription and cloudabi::event, and use that instead of the cloudabi_*_t types.

inline error_or<size_t> poll(range<cloudabi_subscription_t const> in, range<cloudabi_event_t> out) {
	if (out.size() < in.size()) return error::inval;
	size_t n_events;
	if (auto err = cloudabi_sys_poll(in.data(), out.data(), in.size(), &n_events)) {
		return error(err);
	} else {
		return n_events;
	}
}

}
