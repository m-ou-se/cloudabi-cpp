#pragma once

#include <memory>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "cloudabi_types.hpp"
#include "cloudabi_error_or.hpp"
#include "cloudabi_iovec.hpp"
#include "cloudabi_range.hpp"
#include "string_view.hpp"

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

	cloudabi_fd_t number() const { return fd_; }

	friend bool operator == (fd a, fd b) { return a.fd_ == b.fd_; }
	friend bool operator != (fd a, fd b) { return a.fd_ != b.fd_; }

	// cloudabi_sys_fd_ syscalls.

	error_or<void> close() {
		return error(cloudabi_sys_fd_close(fd_));
	}

	static error_or<unique_fd> create1(filetype ft) {
		fd result;
		if (auto err = cloudabi_sys_fd_create1(cloudabi_filetype_t(ft), &result.fd_)) {
			return error(err);
		} else {
			return unique_fd(result);
		}
	}

	static error_or<std::pair<unique_fd, unique_fd>> create2(filetype ft) {
		fd a, b;
		if (auto err = cloudabi_sys_fd_create2(cloudabi_filetype_t(ft), &b.fd_, &b.fd_)) {
			return error(err);
		} else {
			return std::make_pair(unique_fd(a), unique_fd(b));
		}
	}

	error_or<void> datasync() {
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

	error_or<size_t> pread(range<iovec const> iov, filesize offset) {
		size_t n_read;
		if (auto err = cloudabi_sys_fd_pread(fd_, (const cloudabi_iovec_t *)iov.data(), iov.size(), offset, &n_read)) {
			return error(err);
		} else {
			return n_read;
		}
	}
	error_or<size_t> pread(iovec iov, filesize offset) {
		return pread(range<iovec const>(iov), offset);
	}

	error_or<size_t> pwrite(range<ciovec const> iov, filesize offset) {
		size_t n_written;
		if (auto err = cloudabi_sys_fd_pwrite(fd_, (const cloudabi_ciovec_t *)iov.data(), iov.size(), offset, &n_written)) {
			return error(err);
		} else {
			return n_written;
		}
	}
	error_or<size_t> pwrite(ciovec iov, filesize offset) {
		return pwrite(range<ciovec const>{iov}, offset);
	}

	error_or<size_t> read(range<iovec const> iov) {
		size_t n_read;
		if (auto err = cloudabi_sys_fd_read(fd_, (const cloudabi_iovec_t *)iov.data(), iov.size(), &n_read)) {
			return error(err);
		} else {
			return n_read;
		}
	}
	error_or<size_t> read(iovec iov) {
		return read(range<iovec const>(iov));
	}

	error_or<size_t> write(range<ciovec const> iov) {
		size_t n_written;
		if (auto err = cloudabi_sys_fd_write(fd_, (const cloudabi_ciovec_t *)iov.data(), iov.size(), &n_written)) {
			return error(err);
		} else {
			return n_written;
		}
	}
	error_or<size_t> write(ciovec iov) {
		return write(range<ciovec const>(iov));
	}

	error_or<void> replace(fd const & from) {
		return error(cloudabi_sys_fd_replace(from.fd_, fd_));
	}

	error_or<filesize> seek(filedelta offset, whence wh = whence::cur) {
		filesize new_offset;
		if (auto err = cloudabi_sys_fd_seek(fd_, offset, cloudabi_whence_t(wh), &new_offset)) {
			return error(err);
		} else {
			return new_offset;
		}
	}

	error_or<void> stat_get(fdstat & stat) {
		return error(cloudabi_sys_fd_stat_get(fd_, (cloudabi_fdstat_t *)&stat));
	}

	error_or<void> stat_put(fdstat const & stat, fdsflags flags = fdsflags::flags | fdsflags::rights) {
		return error(cloudabi_sys_fd_stat_put(fd_, (cloudabi_fdstat_t *)&stat, cloudabi_fdsflags_t(flags)));
	}

	error_or<void> sync() {
		return error(cloudabi_sys_fd_sync(fd_));
	}

	// cloudabi_sys_mem_map syscall.

	error_or<void *> mem_map(size_t len, filesize off = 0, mprot prot = mprot::read, mflags flags = mflags::private_, void * addr = nullptr) {
		void * mem;
		if (auto err = cloudabi_sys_mem_map(addr, len, cloudabi_mprot_t(prot), cloudabi_mflags_t(flags), fd_, off, &mem)) {
			return error(err);
		} else {
			return mem;
		}
	}

	// cloudabi_sys_file_ syscalls.

	error_or<void> file_advise(filesize offset, filesize len, advice a) {
		return error(cloudabi_sys_file_advise(fd_, offset, len, cloudabi_advice_t(a)));
	}

	error_or<void> file_allocate(filesize offset, filesize len) {
		return error(cloudabi_sys_file_allocate(fd_, offset, len));
	}

	error_or<void> file_create(string_view path, filetype type) {
		return error(cloudabi_sys_file_create(fd_, path.data(), path.size(), cloudabi_filetype_t(type)));
	}

	error_or<unique_fd> file_open(string_view path, oflags flags, fdstat const & init, bool follow_symlinks = true) {
		fd f;
		cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
		if (auto err = cloudabi_sys_file_open(lookup, path.data(), path.size(), cloudabi_oflags_t(flags), (cloudabi_fdstat_t const *)&init, &f.fd_)) {
			return error(err);
		} else {
			return unique_fd(f);
		}
	}

	// TODO: file_readdir

	error_or<size_t> file_readlink(string_view path, range<char> buf) {
		size_t bufused;
		if (auto err = cloudabi_sys_file_readlink(fd_, path.data(), path.size(), buf.data(), buf.size(), &bufused)) {
			return error(err);
		} else {
			return bufused;
		}
	}

	error_or<void> file_stat_fget(filestat & stat) {
		return error(cloudabi_sys_file_stat_fget(fd_, (cloudabi_filestat_t *)&stat));
	}

	error_or<void> file_stat_fput(filestat const & stat, fsflags flags) {
		return error(cloudabi_sys_file_stat_fput(fd_, (cloudabi_filestat_t const *)&stat, cloudabi_fsflags_t(flags)));
	}

	error_or<void> file_stat_get(string_view path, filestat & stat, bool follow_symlinks = true) {
		cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
		return error(cloudabi_sys_file_stat_get(lookup, path.data(), path.size(), (cloudabi_filestat_t *)&stat));
	}

	error_or<void> file_stat_put(string_view path, filestat const & stat, fsflags flags, bool follow_symlinks = true) {
		cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
		return error(cloudabi_sys_file_stat_put(lookup, path.data(), path.size(), (cloudabi_filestat_t const *)&stat, cloudabi_fsflags_t(flags)));
	}

	error_or<void> file_symlink(string_view path, string_view contents) {
		return error(cloudabi_sys_file_symlink(contents.data(), contents.size(), fd_, path.data(), path.size()));
	}

	error_or<void> file_unlink(string_view path, ulflags flags = ulflags::none) {
		return error(cloudabi_sys_file_unlink(fd_, path.data(), path.size(), cloudabi_ulflags_t(flags)));
	}

	// cloudabi_sys_poll_fd syscall.

	error_or<size_t> poll(
		range<subscription const> in,
		range<event> out,
		subscription const & timeout
	) {
		size_t n_events;
		if (auto err = cloudabi_sys_poll_fd(
			fd_, (cloudabi_subscription_t const *)in.data(), in.size(),
			(cloudabi_event_t *)out.data(), out.size(),
			(cloudabi_subscription_t const *)&timeout, &n_events)
		) {
			return error(err);
		} else{
			return n_events;
		}
	}

	// cloudabi_sys_proc_exec syscall.

	error_or<void> proc_exec(range<unsigned char const> data, range<fd const> fds) {
		return error(cloudabi_sys_proc_exec(fd_, data.data(), data.size(), (cloudabi_fd_t *)fds.data(), fds.size()));
	}

	// cloudabi_sys_sock_ syscalls.

	error_or<unique_fd> sock_accept(sockstat & sockstat) {
		fd conn;
		if (auto err = cloudabi_sys_sock_accept(fd_, (cloudabi_sockstat_t *)&sockstat, &conn.fd_)) {
			return error(err);
		} else {
			return unique_fd(conn);
		}
	}

	error_or<void> sock_bind(fd dir, string_view path) {
		return error(cloudabi_sys_sock_bind(fd_, dir.fd_, path.data(), path.size()));
	}

	error_or<void> sock_connect(fd dir, string_view path) {
		return error(cloudabi_sys_sock_connect(fd_, dir.fd_, path.data(), path.size()));
	}

	error_or<void> sock_listen(backlog bl) {
		return error(cloudabi_sys_sock_listen(fd_, bl));
	}

	error_or<void> sock_shutdown(sdflags how) {
		return error(cloudabi_sys_sock_shutdown(fd_, cloudabi_sdflags_t(how)));
	}

	error_or<void> sock_stat_get(sockstat & stat, ssflags flags = ssflags::none) {
		return error(cloudabi_sys_sock_stat_get(fd_, (cloudabi_sockstat_t *)&stat, cloudabi_ssflags_t(flags)));
	}

	friend inline error file_link(fd fd1, string_view path1, fd fd2, string_view path2, bool follow_symlinks = true) {
		cloudabi_lookup_t lookup = {fd1.fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
		return error(cloudabi_sys_file_link(lookup, path1.data(), path1.size(), fd2.fd_, path2.data(), path2.size()));
	}

	friend inline error file_rename(fd fd1, string_view path1, fd fd2, string_view path2) {
		return error(cloudabi_sys_file_rename(fd1.fd_, path1.data(), path1.size(), fd2.fd_, path2.data(), path2.size()));
	}

};

static_assert(sizeof(fd) == sizeof(cloudabi_fd_t), "");
static_assert(alignof(fd) == alignof(cloudabi_fd_t), "");

inline void fd_closer::operator () (fd f) { f.close(); }

inline error_or<void *> mem_map(size_t len, mprot prot = mprot::read | mprot::write, mflags flags = mflags::private_, void * addr = nullptr) {
	return fd(CLOUDABI_MAP_ANON).mem_map(len, 0, prot, flags, addr);
}

}
