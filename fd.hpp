#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error.hpp"
#include "error_or.hpp"

namespace cloudabi {

class fd {
	cloudabi_fd_t fd_ = -1;

public:
	fd() noexcept {}

	fd(fd && other) noexcept : fd_(other.release_fd()) {}

	~fd() { close(); }

	explicit fd(cloudabi_fd_t fd) : fd_(fd) {}

	cloudabi_fd_t release_fd() noexcept {
		auto fd = fd_;
		fd_ = -1;
		return fd;
	}

	fd & operator = (fd && other) noexcept {
		close();
		fd_ = other.release_fd();
		return *this;
	}

	// cloudabi_sys_fd_ syscalls.

	error close() {
		auto err = cloudabi_sys_fd_close(fd_);
		fd_ = -1;
		return error(err);
	}

	static error_or<fd> create_poll() {
		cloudabi_fd_t result;
		if (auto err = cloudabi_sys_fd_create1(CLOUDABI_FILETYPE_POLL, &result)) {
			return error(err);
		} else {
			return fd(result);
		}
	}

	static error_or<std::pair<fd, fd>> create_pipe() {
		cloudabi_fd_t a, b;
		if (auto err = cloudabi_sys_fd_create2(CLOUDABI_FILETYPE_FIFO, &a, &b)) {
			return error(err);
		} else {
			return std::make_pair(fd(a), fd(b));
		}
	}

	error datasync() {
		return error(cloudabi_sys_fd_datasync(fd_));
	}

	error_or<fd> dup() {
		cloudabi_fd_t dup_fd;
		if (auto err = cloudabi_sys_fd_dup(fd_, &dup_fd)) {
			return error(err);
		} else {
			return fd(dup_fd);
		}
	}

	// TODO: More syscalls.


};

}
