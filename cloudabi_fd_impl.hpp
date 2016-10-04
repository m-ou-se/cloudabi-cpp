#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "cloudabi_error_or.hpp"
#include "cloudabi_fd.hpp"
#include "cloudabi_iovec.hpp"
#include "cloudabi_range.hpp"
#include "cloudabi_structs.hpp"
#include "cloudabi_types.hpp"
#include "string_view.hpp"

namespace cloudabi {

inline error_or<void> fd::close() {
	return error(cloudabi_sys_fd_close(fd_));
}

inline  error_or<unique_fd> fd::create1(filetype ft) {
	fd result;
	if (auto err = cloudabi_sys_fd_create1(cloudabi_filetype_t(ft), &result.fd_)) {
		return error(err);
	} else {
		return unique_fd(result);
	}
}

inline  error_or<std::pair<unique_fd, unique_fd>> fd::create2(filetype ft) {
	fd a, b;
	if (auto err = cloudabi_sys_fd_create2(cloudabi_filetype_t(ft), &b.fd_, &b.fd_)) {
		return error(err);
	} else {
		return std::make_pair(unique_fd(a), unique_fd(b));
	}
}

inline error_or<void> fd::datasync() {
	return error(cloudabi_sys_fd_datasync(fd_));
}

inline error_or<unique_fd> fd::dup() {
	fd dup_fd;
	if (auto err = cloudabi_sys_fd_dup(fd_, &dup_fd.fd_)) {
		return error(err);
	} else {
		return unique_fd(dup_fd);
	}
}

inline error_or<size_t> fd::pread(range<iovec const> iov, filesize offset) {
	size_t n_read;
	if (auto err = cloudabi_sys_fd_pread(fd_, (const cloudabi_iovec_t *)iov.data(), iov.size(), offset, &n_read)) {
		return error(err);
	} else {
		return n_read;
	}
}

inline error_or<size_t> fd::pread(iovec iov, filesize offset) {
	return pread(range<iovec const>(iov), offset);
}

inline error_or<size_t> fd::pwrite(range<ciovec const> iov, filesize offset) {
	size_t n_written;
	if (auto err = cloudabi_sys_fd_pwrite(fd_, (const cloudabi_ciovec_t *)iov.data(), iov.size(), offset, &n_written)) {
		return error(err);
	} else {
		return n_written;
	}
}

inline error_or<size_t> fd::pwrite(ciovec iov, filesize offset) {
	return pwrite(range<ciovec const>{iov}, offset);
}

inline error_or<size_t> fd::read(range<iovec const> iov) {
	size_t n_read;
	if (auto err = cloudabi_sys_fd_read(fd_, (const cloudabi_iovec_t *)iov.data(), iov.size(), &n_read)) {
		return error(err);
	} else {
		return n_read;
	}
}

inline error_or<size_t> fd::read(iovec iov) {
	return read(range<iovec const>(iov));
}

inline error_or<size_t> fd::write(range<ciovec const> iov) {
	size_t n_written;
	if (auto err = cloudabi_sys_fd_write(fd_, (const cloudabi_ciovec_t *)iov.data(), iov.size(), &n_written)) {
		return error(err);
	} else {
		return n_written;
	}
}

inline error_or<size_t> fd::write(ciovec iov) {
	return write(range<ciovec const>(iov));
}

inline error_or<void> fd::replace(fd const & from) {
	return error(cloudabi_sys_fd_replace(from.fd_, fd_));
}

inline error_or<filesize> fd::seek(filedelta offset, whence wh) {
	filesize new_offset;
	if (auto err = cloudabi_sys_fd_seek(fd_, offset, cloudabi_whence_t(wh), &new_offset)) {
		return error(err);
	} else {
		return new_offset;
	}
}

inline error_or<fdstat> fd::stat_get() {
	fdstat stat;
	if (auto err = cloudabi_sys_fd_stat_get(fd_, (cloudabi_fdstat_t *)&stat)) {
		return error(err);
	} else {
		return stat;
	}
}

inline error_or<void> fd::stat_put(fdstat const & stat, fdsflags flags) {
	return error(cloudabi_sys_fd_stat_put(fd_, (cloudabi_fdstat_t *)&stat, cloudabi_fdsflags_t(flags)));
}

inline error_or<void> fd::sync() {
	return error(cloudabi_sys_fd_sync(fd_));
}

// cloudabi_sys_mem_map syscall.

inline error_or<void *> fd::mem_map(size_t len, filesize off, mprot prot, mflags flags, void * addr) {
	void * mem;
	if (auto err = cloudabi_sys_mem_map(addr, len, cloudabi_mprot_t(prot), cloudabi_mflags_t(flags), fd_, off, &mem)) {
		return error(err);
	} else {
		return mem;
	}
}

// cloudabi_sys_file_ syscalls.

inline error_or<void> fd::file_advise(filesize offset, filesize len, advice a) {
	return error(cloudabi_sys_file_advise(fd_, offset, len, cloudabi_advice_t(a)));
}

inline error_or<void> fd::file_allocate(filesize offset, filesize len) {
	return error(cloudabi_sys_file_allocate(fd_, offset, len));
}

inline error_or<void> fd::file_create(string_view path, filetype type) {
	return error(cloudabi_sys_file_create(fd_, path.data(), path.size(), cloudabi_filetype_t(type)));
}

inline error_or<unique_fd> fd::file_open(string_view path, oflags flags, fdstat const & init, bool follow_symlinks) {
	fd f;
	cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
	if (auto err = cloudabi_sys_file_open(lookup, path.data(), path.size(), cloudabi_oflags_t(flags), (cloudabi_fdstat_t const *)&init, &f.fd_)) {
		return error(err);
	} else {
		return unique_fd(f);
	}
}

inline error_or<size_t> fd::file_readlink(string_view path, range<char> buf) {
	size_t bufused;
	if (auto err = cloudabi_sys_file_readlink(fd_, path.data(), path.size(), buf.data(), buf.size(), &bufused)) {
		return error(err);
	} else {
		return bufused;
	}
}

inline error_or<filestat> fd::file_stat_fget() {
	filestat stat;
	if (auto err = cloudabi_sys_file_stat_fget(fd_, (cloudabi_filestat_t *)&stat)) {
		return error(err);
	} else {
		return stat;
	}
}

inline error_or<filestat> fd::file_stat_get(string_view path, bool follow_symlinks) {
	filestat stat;
	cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
	if (auto err = cloudabi_sys_file_stat_get(lookup, path.data(), path.size(), (cloudabi_filestat_t *)&stat)) {
		return error(err);
	} else {
		return stat;
	}
}

inline error_or<void> fd::file_stat_fput(filestat const & stat, fsflags flags) {
	return error(cloudabi_sys_file_stat_fput(fd_, (cloudabi_filestat_t const *)&stat, cloudabi_fsflags_t(flags)));
}

inline error_or<void> fd::file_stat_put(string_view path, filestat const & stat, fsflags flags, bool follow_symlinks) {
	cloudabi_lookup_t lookup = {fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
	return error(cloudabi_sys_file_stat_put(lookup, path.data(), path.size(), (cloudabi_filestat_t const *)&stat, cloudabi_fsflags_t(flags)));
}

inline error_or<void> fd::file_symlink(string_view path, string_view contents) {
	return error(cloudabi_sys_file_symlink(contents.data(), contents.size(), fd_, path.data(), path.size()));
}

inline error_or<void> fd::file_unlink(string_view path, ulflags flags) {
	return error(cloudabi_sys_file_unlink(fd_, path.data(), path.size(), cloudabi_ulflags_t(flags)));
}

// cloudabi_sys_poll_fd syscall.

inline error_or<size_t> fd::poll(
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

inline error_or<void> fd::proc_exec(range<unsigned char const> data, range<fd const> fds) {
	return error(cloudabi_sys_proc_exec(fd_, data.data(), data.size(), (cloudabi_fd_t *)fds.data(), fds.size()));
}

// cloudabi_sys_sock_ syscalls.

struct sock_accept_result {
	unique_fd fd;
	sockstat sockstat;
};

inline error_or<sock_accept_result> fd::sock_accept() {
	sockstat stat;
	fd fd;
	if (auto err = cloudabi_sys_sock_accept(fd_, (cloudabi_sockstat_t *)&stat, &fd.fd_)) {
		return error(err);
	} else {
		return sock_accept_result{unique_fd(fd), stat};
	}
}

inline error_or<void> fd::sock_bind(fd dir, string_view path) {
	return error(cloudabi_sys_sock_bind(fd_, dir.fd_, path.data(), path.size()));
}

inline error_or<void> fd::sock_connect(fd dir, string_view path) {
	return error(cloudabi_sys_sock_connect(fd_, dir.fd_, path.data(), path.size()));
}

inline error_or<void> fd::sock_listen(backlog bl) {
	return error(cloudabi_sys_sock_listen(fd_, bl));
}

inline error_or<void> fd::sock_shutdown(sdflags how) {
	return error(cloudabi_sys_sock_shutdown(fd_, cloudabi_sdflags_t(how)));
}

inline error_or<sockstat> fd::sock_stat_get(ssflags flags) {
	sockstat stat;
	if (auto err = cloudabi_sys_sock_stat_get(fd_, (cloudabi_sockstat_t *)&stat, cloudabi_ssflags_t(flags))) {
		return error(err);
	} else {
		return stat;
	}
}

inline error file_link(fd fd1, string_view path1, fd fd2, string_view path2, bool follow_symlinks = true) {
	cloudabi_lookup_t lookup = {fd1.fd_, follow_symlinks ? CLOUDABI_LOOKUP_SYMLINK_FOLLOW : 0u};
	return error(cloudabi_sys_file_link(lookup, path1.data(), path1.size(), fd2.fd_, path2.data(), path2.size()));
}

inline error file_rename(fd fd1, string_view path1, fd fd2, string_view path2) {
	return error(cloudabi_sys_file_rename(fd1.fd_, path1.data(), path1.size(), fd2.fd_, path2.data(), path2.size()));
}

}
