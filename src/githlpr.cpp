#include <algorithm>
#include <filesystem>
#include <functional>
#include <ios>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <cstdlib>

#include "githlpr.hpp"

namespace githlpr
{
	bool has_valid_git_dir_env()
	{
		if (const char *const cgit_dir = std::getenv("GIT_DIR")) {
			return std::filesystem::is_directory(cgit_dir);
		}
		return false;
	}

	void git_cmd_handler::ping(std::ostream& reply)
	{
		reply << replies::ping << std::endl;
	}

	void git_cmd_handler::capabilities(std::ostream& reply)
	{
		for (const std::string_view& cap : replies::caps) {
			reply << cap << std::endl;
		}
	}

	void git_cmd_handler::list(std::ostream&)
	{
	}

	void git_cmd_handler::push(std::ostream&, const std::vector<std::string>&)
	{
	}

	void git_cmd_handler::fetch(std::ostream&, const std::vector<std::string>&)
	{
		throw std::runtime_error("could not parse fetch parameters");
	}

	std::string get_nth_str_word(const std::string_view& str, const size_t n)
	{
		std::istringstream cmdstr{std::string(str)};
		std::string word_n{};
		for (size_t i{}; i < n; i++) {
			word_n.clear();
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
}

namespace githlpr::cmd
{
	const std::array<const std::reference_wrapper<const git_cmd>, 6> useable_cmds{
		blank_line,
		ping,
		caps,
		list,
		push,
		fetch
	};

	std::reference_wrapper<const git_cmd> get_cmd(const std::string_view& prefix, const std::array<const std::reference_wrapper<const git_cmd>, 6>& cmd_list, const git_cmd& not_found_fallback)
	{
		auto find = std::find_if(
				cmd_list.cbegin(),
				cmd_list.cend(),
				[prefix](const auto& v) {
					return prefix == v.get().prefix;
				});
		if (find == cmd_list.cend()) {
			return std::cref(not_found_fallback);
		}
		return *find;
	}

	std::ostream& operator<<(std::ostream& strm, const git_cmd cmd)
	{
		strm << cmd.prefix;
		return strm;
	}

	bool git_cmd::operator==(const git_cmd& other) const
	{
		return id == other.id;
	}

	bool git_cmd::operator!=(const git_cmd& other) const
	{
		return not operator==(other);
	}

	git_cmd::operator std::string_view() const
	{
		return prefix;
	}

	git_cmd::operator std::string() const
	{
		return std::string(prefix);
	}
}
