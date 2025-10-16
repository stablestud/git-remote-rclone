#include <filesystem>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cstdlib>

#include "githlpr.hpp"
#include "utils.hpp"

namespace githlpr::cmds
{
	const std::string caps{"capabilities"};
	const std::string push{"push"};
	const std::string ping{"ping"};
}

const std::string githlpr::capabilities{"push"};
const std::string githlpr::ping_reply{"pong"};

bool githlpr::has_valid_git_dir_env()
{
	if (const char *const cgit_dir = std::getenv("GIT_DIR")) {
		return std::filesystem::is_directory(cgit_dir);
	}
	return false;
}

enum git_cmd_t {
	CAPABILITIES,
	PING,
	PUSH,
	UNKNOWN,
	ENDL
};

static std::string get_nth_str_word(const std::string_view& str, const size_t n)
{
	std::istringstream cmdstr{std::string(str)};
	std::string word_n{};
	for (size_t i{}; i < n; i++) {
		cmdstr >> std::skipws >> word_n;
	}
	return word_n;
}

static std::string get_push_dst(const std::string_view& push_args)
{
	size_t colon_pos{push_args.find(':')};
	if (std::string_view::npos == colon_pos) {
		throw std::runtime_error("could not parse dst from push arguments");
	}
	colon_pos += 1;
	return get_nth_str_word(push_args.substr(colon_pos, push_args.length() - colon_pos), 1);
}

static git_cmd_t get_cmd_type(const std::string_view& cmd)
{
	if        (cmd == githlpr::cmds::caps) {
		return git_cmd_t::CAPABILITIES;
	} else if (cmd == githlpr::cmds::push) {
		return git_cmd_t::PUSH;
	} else if (cmd == githlpr::cmds::ping) {
		return git_cmd_t::PING;
	} else if (cmd.empty()) {
		return git_cmd_t::ENDL;
	} else {
		return git_cmd_t::UNKNOWN;
	}
}

bool githlpr::process_git_cmds(std::istream& input, std::ostream& output)
{
	bool unknwn_cmd = false;
	bool error = true;
	std::string cmd;
	std::stringstream reply_buf{};
	bool in_block = false;
	while(not std::getline(input, cmd).eof()) {
		std::string cmd_prefix = get_nth_str_word(cmd, 1);
		switch(get_cmd_type(cmd_prefix)) {
			case git_cmd_t::CAPABILITIES:
				in_block = true;
				reply_buf << "push" << std::endl;
				break;
			case git_cmd_t::PUSH:
				in_block = true;
				reply_buf << "ok " << get_push_dst(get_nth_str_word(cmd, 2)) << std::endl;
				break;
			case git_cmd_t::PING:
				in_block = true;
				reply_buf << ping_reply << std::endl;
				break;
			case git_cmd_t::ENDL:
				if (in_block) {
					reply_buf << std::endl;
					output << reply_buf.str();
					reply_buf.clear();
					reply_buf.str("");
					error = false;
					in_block = false;
				}
				break;
			default:
				unknwn_cmd = true;
		}
	}
	return unknwn_cmd or error;
}
