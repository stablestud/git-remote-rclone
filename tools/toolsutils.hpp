#ifndef TOOLSUTILS_HPP
#define TOOLSUTILS_HPP

#include <array>
#include <iostream>
#include <memory>

#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace toolsutils
{
	constexpr sockaddr_un addr{
		AF_UNIX,
		"\0git-remote-interactive-proto"
	};

	std::string self;

	[[noreturn]] void exit_errno(const std::string& msg)
	{
		std::cerr << self << ": " << msg << ": " << std::strerror(errno) <<  std::endl;
		std::exit(EXIT_FAILURE);
	}

	class fd_t {
		int id;
		mutable std::shared_ptr<int> shared;
	public:
		explicit fd_t(const int id) : id(id), shared(&this->id, [](const int* id) { ::close(*id); }) {}

		int get() const
		{
			return *shared.get();
		}

		void close() const
		{
			shared.reset();
		}
	};

	fd_t stdin_fd{STDIN_FILENO};
	fd_t stdout_fd{STDOUT_FILENO};

	class pipe_t {
		const fd_t read;
		const fd_t write;
		pipe_t(const pipe_t&) = delete;
		pipe_t& operator=(const pipe_t&) = delete;
	public:
		explicit pipe_t(const int read, const int write) : read(read), write(write) {}

		int get_read_fd() const
		{
			return read.get();
		}

		int get_write_fd() const
		{
			return write.get();
		}
	};

	pipe_t create_pipe()
	{
		std::array<int, 2> fds{};
		if (-1 == ::pipe(fds.data())) {
			exit_errno("cannot create pipe");
		}
		return pipe_t(fds.at(0), fds.at(1));
	}

	fd_t create_socket()
	{
		if (const int sock = ::socket(AF_UNIX, SOCK_STREAM, 0); sock != -1) {
			return fd_t(sock);
		}
		exit_errno("cannot create socket");
	}

	bool fd_forward(const fd_t& from, const fd_t& to)
	{
		const pipe_t pipe{create_pipe()};
		ssize_t bytes;
		if (bytes = ::splice(from.get(), NULL, pipe.get_write_fd(), NULL, 1024, SPLICE_F_MORE | SPLICE_F_MOVE); -1 == bytes) {
			exit_errno("cannot read data from source file descriptor");
		} else if (bytes == 0) {
			return true; // EOF reached
		}
		if (bytes = ::splice(pipe.get_read_fd(), NULL, to.get(), NULL, bytes, SPLICE_F_MORE | SPLICE_F_MOVE); -1 == bytes) {
			exit_errno("cannot write data to target file descriptor");
		}
		return false;
	}

	void transfer_loop(const fd_t& socket, const fd_t& input, const fd_t& output)
	{
		constexpr int SOCKET_FD = 0; 
		constexpr int INPUT_FD  = 1; 

		std::array<pollfd, 2> fds{
			pollfd{socket.get(), POLLIN, 0},
			pollfd{input.get(),  POLLIN, 0}
		};

		bool eof = false;
		while (not eof) {
			if (-1 == ::poll(fds.data(), 2, -1)) {
				exit_errno("failed to poll file descriptors");
			}
			if (fds[SOCKET_FD].revents & POLLIN) {
				eof |= fd_forward(socket, output);
			}
			if (fds[INPUT_FD].revents & POLLIN) {
				eof |= fd_forward(input, socket);
			}
		}
	}
}

#endif /* TOOLSUTILS_HPP */
