#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef DEBUG

#define DEBUGGER (debug::stop())
#define DEBUG_WAIT (debug::wait_loop())
#define DEBUG_LOG(x) (void)debug::log(__PRETTY_FUNCTION__, (x))

#include <iostream>
#include <string_view>
#include <csignal>
#include <unistd.h>

namespace debug
{
	void wait_loop() // Infinite while loop; used if raising SIGSTOP does not work (terminated by parent)
	{		 // Use debugger to exit while loop, by setting var exit to true
		std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
		volatile bool exit = false;
		while (not exit);
	}

	void stop()  // Raise SIGSTOP to self; halts execution to be able to attach debugger at current execution
	{
		std::cerr << __PRETTY_FUNCTION__ << ": attach debugger to pid: " << getpid() << std::endl;
		std::raise(SIGSTOP);
	}

	void log(const std::string_view& file, const std::string_view& msg)
	{
		std::cerr << file << ": " << msg << std::endl;
	}
}

#else

#define DEBUGGER
#define DEBUG_WAIT
#define DEBUG_LOG(x) (void)(x)

#endif /* DEBUG */
#endif /* DEBUG_HPP */
