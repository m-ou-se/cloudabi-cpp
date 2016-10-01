#pragma once

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "cloudabi_error_or.hpp"
#include "cloudabi_range.hpp"
#include "cloudabi_types.hpp"

namespace cloudabi {

inline error_or<timestamp> clock_res_get(clockid clock_id) {
	cloudabi_timestamp_t resolution;
	if (auto err = cloudabi_sys_clock_res_get(cloudabi_clockid_t(clock_id), &resolution)) {
		return error(err);
	} else {
		return timestamp(resolution);
	}
}

inline error_or<timestamp> clock_time_get(clockid clock_id, timestamp precision = 0) {
	cloudabi_timestamp_t time;
	if (auto err = cloudabi_sys_clock_time_get(cloudabi_clockid_t(clock_id), cloudabi_timestamp_t(precision), &time)) {
		return error(err);
	} else {
		return timestamp(time);
	}
}

}
