#ifndef GITHLPR_HPP
#define GITHLPR_HPP

namespace githlpr
{
	namespace cmds
	{
		extern const std::string caps;
		extern const std::string push;
		extern const std::string ping; // not a git helper cmd; implemented for testing
	}

	extern const std::string capabilities;
	extern const std::string ping_reply;

	extern bool has_valid_git_dir_env();
	extern bool process_git_cmds(std::istream&, std::ostream&);
}

#endif /* GITHLPR_HPP */
