#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef DEBUG

#include <iostream>
#include <csignal>
#include <unistd.h>

#define DEBUGGER   (debug::debug_stop())
#define DEBUG_WAIT (debug::wait_loop())

namespace debug
{
	void wait_loop(); // Infinite while loop; used if raising SIGSTOP does not work (terminated by parent)
			  // Use debugger to exit while loop, by setting var exit to true
	void debug_stop();  // Raise SIGSTOP to self; halts execution to be able to attach debugger at current execution
}

void debug::debug::wait_loop()
{
	std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
	volatile bool exit = false;
	while (not exit);
}

void debug::debug::debug_stop()
{
	std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
	std::raise(SIGSTOP);
}

#endif

#endif /* DEBUG_HPP */
