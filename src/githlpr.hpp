#ifndef GITHLPR_HPP
#define GITHLPR_HPP

#include <iostream>
#include <string>

namespace githlpr
{
	namespace cmds
	{
		inline constexpr std::string_view caps{"capabilities"};
		inline constexpr std::string_view push{"push"};
		inline constexpr std::string_view ping{"ping"}; // not a git helper cmd; implemented for testing
	}

	namespace replies
	{
		inline constexpr std::string_view capabilities{"push"};
		inline constexpr std::string_view ping_reply{"pong"};
	}

	extern bool has_valid_git_dir_env();
	extern void process_git_cmds(std::istream&, std::ostream&);
}

#endif /* GITHLPR_HPP */
