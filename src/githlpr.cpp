#include <filesystem>
#include <ios>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>

#include <cstdlib>

#include "debug.hpp"
#include "githlpr.hpp"

namespace
{
	enum class git_cmd_t {
		CAPABILITIES,
		PING,
		PUSH,
		LIST,
		UNKNOWN,
		ENDL
	};

	std::string get_nth_str_word(const std::string_view& str, const size_t n)
	{
		std::istringstream cmdstr{std::string(str)};
		std::string word_n{};
		for (size_t i{}; i < n; i++) {
			cmdstr >> std::skipws >> word_n;
		}
		return word_n;
	}

	std::string get_push_dst(const std::string_view& push_args)
	{
		size_t colon_pos{push_args.find(':')};
		if (std::string_view::npos == colon_pos) {
			throw std::runtime_error("could not parse dst-ref from push argument");
		}
		colon_pos += 1;
		return get_nth_str_word(push_args.substr(colon_pos, push_args.length() - colon_pos), 1);
	}

	git_cmd_t get_cmd_type(const std::string_view& cmd)
	{
		if        (cmd == githlpr::cmds::caps) {
			return git_cmd_t::CAPABILITIES;
		} else if (cmd == githlpr::cmds::push) {
			return git_cmd_t::PUSH;
		} else if (cmd == githlpr::cmds::list) {
			return git_cmd_t::LIST;
		} else if (cmd == githlpr::cmds::ping) {
			return git_cmd_t::PING;
		} else if (cmd.empty()) {
			return git_cmd_t::ENDL;
		} else {
			return git_cmd_t::UNKNOWN;
		}
	}
}

bool githlpr::has_valid_git_dir_env()
{
	if (const char *const cgit_dir = std::getenv("GIT_DIR")) {
		return std::filesystem::is_directory(cgit_dir);
	}
	return false;
}

void githlpr::process_git_cmds(std::istream& input, std::ostream& output)
{
	std::string cmd;
	while(not std::getline(input, cmd).eof()) {
		std::string cmd_prefix = get_nth_str_word(cmd, 1);
		DEBUG_LOG(">> " + cmd);
		switch(get_cmd_type(cmd_prefix)) {
			case git_cmd_t::CAPABILITIES:
				DEBUG_LOG("<< my capabilities");
				output << replies::capabilities << std::endl << std::endl;
				break;
			case git_cmd_t::PUSH:
				DEBUG_LOG("<< push ok");
				output << "ok " << get_push_dst(get_nth_str_word(cmd, 2)) << std::endl << std::endl;
				break;
			case git_cmd_t::LIST:
				DEBUG_LOG("<< refs");
				output << "2a569a9e9e5a0d8e4ce829bbdd84904633024f86 refs/heads/master" << std::endl << std::endl;
				break;
			case git_cmd_t::PING:
				DEBUG_LOG("<< pong");
				output << replies::ping_reply << std::endl << std::endl;
				break;
			case git_cmd_t::ENDL:
				DEBUG_LOG("end-of-cmd, terminating");
				return;
			default:
				DEBUG_LOG("unknown cmd");
				throw std::runtime_error("unknown command: " + cmd);
		}
	}
}
