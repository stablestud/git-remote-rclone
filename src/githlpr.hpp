#ifndef GITHLPR_HPP
#define GITHLPR_HPP

#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "debug.hpp"

namespace githlpr
{
	namespace cmd
	{
		enum class git_cmd_type {
			unknown,
			blank_line,
			ping,
			caps,
			list,
			push,
			fetch
		};

		struct git_cmd {
			const std::string_view prefix;
			const git_cmd_type id;
			const bool is_batch;
			constexpr git_cmd(const std::string_view cmd = "", const git_cmd_type id = git_cmd_type::unknown, const bool is_batch = false) : prefix(cmd), id(id), is_batch(is_batch) {};
			constexpr git_cmd(const git_cmd& copy) : git_cmd(copy.prefix, copy.id, copy.is_batch) {};
			bool operator==(const git_cmd&) const;
			bool operator!=(const git_cmd&) const;
			operator std::string_view() const;
			operator std::string() const;
		};

		extern const std::array<const std::reference_wrapper<const git_cmd>, 6> useable_cmds;

		std::ostream& operator<<(std::ostream&, const git_cmd);

		constexpr git_cmd unknown{};
		constexpr git_cmd blank_line{"", git_cmd_type::blank_line, false};
		constexpr git_cmd ping{"ping", git_cmd_type::ping, false}; // not a real git helper cmd; implemented for testing
		constexpr git_cmd caps{"capabilities", git_cmd_type::caps, false};
		constexpr git_cmd list{"list", git_cmd_type::list, false};
		constexpr git_cmd push{"push", git_cmd_type::push, true};
		constexpr git_cmd fetch{"fetch", git_cmd_type::fetch, true};

		std::reference_wrapper<const git_cmd> get_cmd(const std::string_view&, const std::array<const std::reference_wrapper<const git_cmd>, 6>& = useable_cmds, const git_cmd& = unknown);
	}

	namespace replies
	{
		inline constexpr std::array<std::string_view, 2> caps{{"push", "fetch"}};
		inline constexpr std::string_view ping{"pong"};
	}

	bool has_valid_git_dir_env();
	std::string get_nth_str_word(const std::string_view&, const size_t);
	std::string get_push_dst(const std::string_view&);

	struct git_cmd_handler {
		static void ping(std::ostream&);
		static void capabilities(std::ostream&);
		static void list(std::ostream&);
		static void push(std::ostream&, const std::vector<std::string>&);
		static void fetch(std::ostream&, const std::vector<std::string>&);
	};

	template<typename GitCmdHandler = git_cmd_handler>
	void process_git_line_cmds(std::istream& input, std::ostream& output)
	{
		std::string cmd_line;
		std::reference_wrapper<const cmd::git_cmd> last_cmd = std::ref(cmd::unknown);
		std::vector<std::string> refspecs{};
		while(std::getline(input, cmd_line).good()) {
			DEBUG_LOG(">> " + cmd_line);
			const std::string cmd_prefix = get_nth_str_word(cmd_line, 1);
			const std::reference_wrapper<const cmd::git_cmd> cmd = cmd::get_cmd(cmd_prefix);
			if ((cmd.get() not_eq last_cmd.get()) and (cmd.get() not_eq cmd::blank_line) and last_cmd.get().is_batch) {
				throw std::runtime_error("last batch command not terminated");
			}
			std::stringstream reply{};
			switch(cmd.get().id) {
			case cmd::git_cmd_type::ping:
				GitCmdHandler::ping(reply);
				break;
			case cmd::git_cmd_type::caps:
				GitCmdHandler::capabilities(reply);
				break;
			case cmd::git_cmd_type::list:
				GitCmdHandler::list(reply);
				//reply << "2a569a9e9e5a0d8e4ce829bbdd84904633024f86 refs/heads/master" << std::endl;
				break;
			case cmd::git_cmd_type::push:
			case cmd::git_cmd_type::fetch:
				{
					const std::string refspec = get_nth_str_word(cmd_line, 2);
					if (not refspec.empty()) {
						refspecs.push_back(refspec);
					}
				}
				//reply << "ok " << get_push_dst(get_nth_str_word(cmd, 2)) << std::endl;
				break;
			case cmd::git_cmd_type::blank_line:
				switch(last_cmd.get().id) {
				case cmd::git_cmd_type::fetch:
					GitCmdHandler::fetch(reply, refspecs);
					break;
				case cmd::git_cmd_type::push:
					GitCmdHandler::push(reply, refspecs);
					break;
				default:
					last_cmd = cmd;
					continue;
				}
				refspecs.clear();
				break;
			default:
				DEBUG_LOG("unknown cmd");
				throw std::runtime_error("unknown command: " + cmd_prefix);
			}
			if (not cmd.get().is_batch) {
				DEBUG_LOG("<< " + reply.str());
				output << reply.str() << std::endl;
			}
			last_cmd = cmd;
		}
		if (last_cmd.get().is_batch) {
			throw std::runtime_error("last batch command not terminated");
		}
	}
}

#endif /* GITHLPR_HPP */
