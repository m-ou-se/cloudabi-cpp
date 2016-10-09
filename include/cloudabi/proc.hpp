#pragma once

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error_or.hpp"
#include "fd.hpp"
#include "types.hpp"

namespace cloudabi {

[[noreturn]] inline void proc_exit(exitcode rval) {
	cloudabi_sys_proc_exit(rval);
}

struct fork_result {
	unique_fd fd;
	tid tid;
};

inline error_or<fork_result> proc_fork() {
	cloudabi_fd_t f;
	cloudabi_tid_t t;
	if (auto err = cloudabi_sys_proc_fork(&f, &t)) {
		return error(err);
	} else {
		return fork_result{unique_fd(fd(f)), tid(t)};
	}
}

inline error_or<void> proc_raise(signal sig) {
	return error(cloudabi_sys_proc_raise(cloudabi_signal_t(sig)));
}

}
