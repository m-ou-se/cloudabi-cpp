#include <memory>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error.hpp"
#include "error_or.hpp"
#include "range.hpp"
#include "string_view.hpp"
#include "types.hpp"

namespace cloudabi {

struct fd;

struct fd_closer {
	using pointer = fd;
	void operator () (fd f);
};

using unique_fd = std::unique_ptr<fd, fd_closer>;

struct fd {

private:
	cloudabi_fd_t fd_ = -1;

public:

	fd() noexcept {}
	fd(std::nullptr_t) noexcept {}
	explicit fd(cloudabi_fd_t fd) noexcept : fd_(fd) {}

	explicit operator bool() const { return fd_ != cloudabi_fd_t(-1); }

	friend bool operator == (fd a, fd b) { return a.fd_ == b.fd_; }
	friend bool operator != (fd a, fd b) { return a.fd_ != b.fd_; }

	// cloudabi_sys_fd_ syscalls.

	error close() {
		return error(cloudabi_sys_fd_close(fd_));
	}

	static error_or<unique_fd> create1() {
		fd result;
		if (auto err = cloudabi_sys_fd_create1(CLOUDABI_FILETYPE_POLL, &result.fd_)) {
			return error(err);
		} else {
			return unique_fd(result);
		}
	}

	static error_or<std::pair<unique_fd, unique_fd>> create_pipe() {
		fd a, b;
		if (auto err = cloudabi_sys_fd_create2(CLOUDABI_FILETYPE_FIFO, &b.fd_, &b.fd_)) {
			return error(err);
		} else {
			return std::make_pair(unique_fd(a), unique_fd(b));
		}
	}

	error datasync() {
		return error(cloudabi_sys_fd_datasync(fd_));
	}

	error_or<unique_fd> dup() {
		fd dup_fd;
		if (auto err = cloudabi_sys_fd_dup(fd_, &dup_fd.fd_)) {
			return error(err);
		} else {
			return unique_fd(dup_fd);
		}
	}

	error replace(fd const & from) {
		return error(cloudabi_sys_fd_replace(from.fd_, fd_));
	}

	error sync() {
		return error(cloudabi_sys_fd_sync(fd_));
	}

	// cloudabi_sys_file_ syscalls.

	error file_allocate(filesize_t offset, filesize_t len) {
		return error(cloudabi_sys_file_allocate(fd_, offset, len));
	}

	error file_create(string_view path, cloudabi_filetype_t type) {
		return error(cloudabi_sys_file_create(fd_, path.data(), path.size(), type));
	}

	error file_symlink(string_view path, string_view contents) {
		return error(cloudabi_sys_file_symlink(contents.data(), contents.size(), fd_, path.data(), path.size()));
	}

	error file_unlink(string_view path, cloudabi_ulflags_t flags = 0) {
		return error(cloudabi_sys_file_unlink(fd_, path.data(), path.size(), flags));
	}

	// cloudabi_sys_poll_fd syscall.
	
	error_or<size_t> poll(
		range<cloudabi_subscription_t const> in,
		range<cloudabi_event_t> out,
		cloudabi_subscription_t const & timeout
	) {
		size_t n_events;
		if (auto err = cloudabi_sys_poll_fd(fd_, in.data(), in.size(), out.data(), out.size(), &timeout, &n_events)) {
			return error(err);
		} else{
			return n_events;
		}
	}

	// cloudabi_sys_proc_exec syscall.

	error proc_exec(range<unsigned char const> data, range<fd const> fds) {
		return error(cloudabi_sys_proc_exec(fd_, data.data(), data.size(), (cloudabi_fd_t *)fds.data(), fds.size()));
	}

	// cloudabi_sys_sock_ syscalls.

	error_or<unique_fd> sock_accept(cloudabi_sockstat_t & sockstat) {
		fd conn;
		if (auto err = cloudabi_sys_sock_accept(fd_, &sockstat, &conn.fd_)) {
			return error(err);
		} else {
			return unique_fd(conn);
		}
	}

	error sock_bind(fd dir, string_view path) {
		return error(cloudabi_sys_sock_bind(fd_, dir.fd_, path.data(), path.size()));
	}

	error sock_connect(fd dir, string_view path) {
		return error(cloudabi_sys_sock_connect(fd_, dir.fd_, path.data(), path.size()));
	}

	error sock_listen(cloudabi_backlog_t backlog) {
		return error(cloudabi_sys_sock_listen(fd_, backlog));
	}

	error sock_shutdown(cloudabi_sdflags_t how) {
		return error(cloudabi_sys_sock_shutdown(fd_, how));
	}

	error sock_stat_get(cloudabi_sockstat_t & stat, cloudabi_ssflags_t flags = 0) {
		return error(cloudabi_sys_sock_stat_get(fd_, &stat, flags));
	}

	// TODO: More syscalls.

};

inline void fd_closer::operator () (fd f) { f.close(); }

}
