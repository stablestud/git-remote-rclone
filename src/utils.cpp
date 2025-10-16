#include "utils.hpp"

#ifdef DEBUG

#include <iostream>
#include <csignal>
#include <unistd.h>

void utils::debug::wait_loop()
{
	std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
	volatile bool exit = false;
	while (not exit);
}

void utils::debug::debug_stop()
{
	std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
	std::raise(SIGSTOP);
}

#endif
