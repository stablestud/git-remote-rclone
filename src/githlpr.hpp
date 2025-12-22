#ifndef GITHLPR_HPP
#define GITHLPR_HPP

#include <array>
#include <iostream>
#include <string>

#include "debug.hpp"

namespace githlpr
{
	namespace cmds
	{
		inline constexpr std::string_view caps{"capabilities"};
		inline constexpr std::string_view push{"push"};
		inline constexpr std::string_view list{"list"};
		inline constexpr std::string_view fetch{"fetch"};
		inline constexpr std::string_view ping{"ping"}; // not a git helper cmd; implemented for testing
	}

	namespace replies
	{
		inline constexpr std::array<std::string_view, 2> caps{{"push", "fetch"}};
		inline constexpr std::string_view ping_reply{"pong"};
	}

	extern bool has_valid_git_dir_env();

	enum class git_cmd_t {
		CAPABILITIES,
		PING,
		PUSH,
		LIST,
		UNKNOWN,
		BLANK_LINE
	};

	std::string get_nth_str_word(const std::string_view&, const size_t);
	std::string get_push_dst(const std::string_view&);
	git_cmd_t get_cmd_type(const std::string_view&);
	void write_caps(std::ostream&);

	struct git_cmd_parser {
	};

	template<typename GitCmdParsePolicy = git_cmd_parser>
	void process_git_cmds(std::istream& input, std::ostream& output)
	{
		std::string cmd;
		while(not std::getline(input, cmd).eof()) {
			std::string cmd_prefix = get_nth_str_word(cmd, 1);
			std::stringstream reply{};
			DEBUG_LOG(">> " + cmd);
			switch(get_cmd_type(cmd_prefix)) {
				case git_cmd_t::CAPABILITIES:
					write_caps(reply);
					break;
				case git_cmd_t::PUSH:
					reply << "ok " << get_push_dst(get_nth_str_word(cmd, 2)) << std::endl;
					break;
				case git_cmd_t::LIST:
					reply << "2a569a9e9e5a0d8e4ce829bbdd84904633024f86 refs/heads/master" << std::endl;
					break;
				case git_cmd_t::PING:
					reply << replies::ping_reply << std::endl;
					break;
				case git_cmd_t::BLANK_LINE:
					break;
				default:
					DEBUG_LOG("unknown cmd");
					throw std::runtime_error("unknown command: " + cmd);
			}
			DEBUG_LOG("<< " + reply.str());
			if (std::stringstream::traits_type::eof() != reply.peek()) {
				output << reply.str() << std::endl;
			}
		}
	}
}

#endif /* GITHLPR_HPP */
