#include <array>
#include <filesystem>
#include <iostream>
#include <thread>

#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "toolsutils.hpp"

namespace
{
	int listen_socket(const toolsutils::fd_t& sock)
	{
		if (-1 == ::bind(sock.get(), reinterpret_cast<const sockaddr*>(&toolsutils::addr), sizeof(toolsutils::addr))) {
			toolsutils::exit_errno("cannot bind socket");
		}
		if (-1 == ::listen(sock.get(), 16)) {
			toolsutils::exit_errno("cannot listen on socket");
		}
		if (const int client_fd = ::accept(sock.get(), nullptr, nullptr); -1 != client_fd) {
			return client_fd;
		}
		toolsutils::exit_errno("cannot accept on socket");
	}
}

int main(int argc, char* argv[])
{
	toolsutils::self = std::filesystem::path(argv[0]).filename();
	toolsutils::fd_t socket = toolsutils::create_socket();
	const toolsutils::fd_t client{listen_socket(socket)};
	toolsutils::transfer_loop(client, toolsutils::stdin_fd, toolsutils::stdout_fd);
}
