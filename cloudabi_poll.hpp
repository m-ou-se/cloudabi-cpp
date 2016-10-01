#pragma once

#include <atomic>
#include <type_traits>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "cloudabi_types.hpp"
#include "cloudabi_error_or.hpp"
#include "cloudabi_range.hpp"

namespace cloudabi {

inline error_or<size_t> poll(range<subscription const> in, range<cloudabi_event_t> out) {
	if (out.size() < in.size()) return error::inval;
	size_t n_events;
	if (auto err = cloudabi_sys_poll((cloudabi_subscription_t const *)in.data(), out.data(), in.size(), &n_events)) {
		return error(err);
	} else {
		return n_events;
	}
}

}
