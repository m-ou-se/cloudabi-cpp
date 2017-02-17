#pragma once

#include <initializer_list>
#include <utility>

#include <mstd/range.hpp>
#include <mstd/string_view.hpp>
#include <mstd/unique.hpp>

#include <cloudabi_types.h>
#include <cloudabi_syscalls.h>

#include "error_or.hpp"
#include "iovec.hpp"
#include "types.hpp"

namespace cloudabi {

using std::size_t;

using mstd::range;
using mstd::string_view;
using mstd::unique;

struct fd;

struct fd_closer {
	void operator () (fd f);
};

using unique_fd = unique<fd, fd_closer>;

struct sock_accept_result;

struct fd {

private:
	cloudabi_fd_t fd_ = -1;

public:
	fd() noexcept {}

	explicit fd(cloudabi_fd_t fd) noexcept : fd_(fd) {}

	explicit operator bool() const { return fd_ != cloudabi_fd_t(-1); }

	cloudabi_fd_t number() const { return fd_; }

	friend bool operator == (fd a, fd b) { return a.fd_ == b.fd_; }
	friend bool operator != (fd a, fd b) { return a.fd_ != b.fd_; }

	// cloudabi_sys_fd_ syscalls.

	error_or<void> close();

	static error_or<unique_fd> create1(filetype);

	static error_or<std::pair<unique_fd, unique_fd>> create2(filetype);

	error_or<void> datasync();

	error_or<unique_fd> dup();

	error_or<size_t> pread(range<iovec const>, filesize offset);
	error_or<size_t> pread(iovec, filesize offset);

	error_or<size_t> pwrite(range<ciovec const>, filesize offset);
	error_or<size_t> pwrite(ciovec, filesize offset);

	error_or<size_t> read(range<iovec const>);
	error_or<size_t> read(std::initializer_list<iovec>);
	error_or<size_t> read(iovec);

	error_or<size_t> write(range<ciovec const>);
	error_or<size_t> write(std::initializer_list<ciovec>);
	error_or<size_t> write(ciovec);

	error_or<void> replace(fd const & from);

	error_or<filesize> seek(filedelta offset, whence = whence::cur);

	error_or<fdstat> stat_get();

	error_or<void> stat_put(fdstat const &, fdsflags = fdsflags::flags | fdsflags::rights);

	error_or<void> sync();

	// cloudabi_sys_mem_map syscall.

	error_or<void *> mem_map(size_t len, filesize off = 0, mprot = mprot::read, mflags = mflags::private_, void * addr = nullptr);

	// cloudabi_sys_file_ syscalls.

	error_or<void> file_advise(filesize offset, filesize len, advice);

	error_or<void> file_allocate(filesize offset, filesize len);

	error_or<void> file_create(string_view path, filetype);

	error_or<unique_fd> file_open(
		string_view path,
		rights base_rights,
		oflags = oflags::none,
		fdflags = fdflags::none,
		rights inheriting_rights = rights::none,
		bool follow_symlinks = true
	);

	// TODO: file_readdir

	error_or<size_t> file_readlink(string_view path, range<char> buf);

	error_or<filestat> file_stat_fget();

	error_or<filestat> file_stat_get (string_view path, bool follow_symlinks = true);

	error_or<void> file_stat_fput(filestat const &, fsflags);

	error_or<void> file_stat_put(string_view path, filestat const &, fsflags, bool follow_symlinks = true);

	error_or<void> file_symlink(string_view path, string_view contents);

	error_or<void> file_unlink(string_view path, ulflags = ulflags::none);

	// cloudabi_sys_poll_fd syscall.

	error_or<size_t> poll(
		range<subscription const> in,
		range<event> out,
		subscription const & timeout
	);

	// cloudabi_sys_proc_exec syscall.

	error_or<void> proc_exec(range<unsigned char const> data, range<fd const> fds);

	// cloudabi_sys_sock_ syscalls.

	error_or<sock_accept_result> sock_accept();

	error_or<void> sock_bind(fd dir, string_view path);

	error_or<void> sock_connect(fd dir, string_view path);

	error_or<void> sock_listen(backlog);

	error_or<void> sock_shutdown(sdflags);

	error_or<sockstat> sock_stat_get(ssflags = ssflags::none);

	friend inline error file_link(fd fd1, string_view path1, fd fd2, string_view path2, bool follow_symlinks);

	friend inline error file_rename(fd fd1, string_view path1, fd fd2, string_view path2);

};

static_assert(sizeof(fd) == sizeof(cloudabi_fd_t), "");
static_assert(alignof(fd) == alignof(cloudabi_fd_t), "");

inline void fd_closer::operator () (fd f) { f.close(); }

}

