#ifndef UTILS_HPP
#define UTILS_HPP

#ifdef DEBUG
	#define DEBUGGER   (utils::debug::debug_stop())
	#define DEBUG_WAIT (utils::debug::wait_loop())
#endif

namespace utils
{
#ifdef DEBUG
	namespace debug
	{
		void wait_loop(); // Infinite while loop; used if raising SIGSTOP does not work (terminated by parent)
				  // Use debugger to exit while loop, by setting var exit to true
		void debug_stop();  // Raise SIGSTOP to self; halts execution to be able to attach debugger at current execution
	}
#endif
}

#endif /* UTILS_HPP */
