#include <filesystem>
#include <thread>

#include <sys/socket.h>

#include "toolsutils.hpp"

namespace
{
	void connect_socket_abstr(const toolsutils::fd_t& sock)
	{
		if (-1 == ::connect(sock.get(), reinterpret_cast<const sockaddr*>(&toolsutils::addr), sizeof(toolsutils::addr))) {
			toolsutils::exit_errno("cannot connect socket");
		}
	}
}

int main(const int /* argc */, const char *const argv[])
{
	toolsutils::self = std::filesystem::path(argv[0]).filename();
	toolsutils::fd_t socket = toolsutils::create_socket();
	connect_socket_abstr(socket);
	toolsutils::transfer_loop(socket, toolsutils::stdin_fd, toolsutils::stdout_fd);
}
