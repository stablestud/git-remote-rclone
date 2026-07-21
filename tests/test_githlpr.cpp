#include <chrono>
#include <functional>
#include <future>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <cstdlib>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "testutils.hpp"

#include "githlpr.hpp"

namespace
{
	const std::string_view test_sha1 = "2a569a9e9e5a0d8e4ce829bbdd84904633024f86";
	const std::string_view test_ref = "refs/heads/master";

	bool is_ping_reply(std::istream& strm)
	{
		return githlpr::replies::ping == testutils::getline(strm);
	}

	bool is_caps_reply(std::istream& strm)
	{
		std::string reply;
		for (const std::string_view& cap : githlpr::replies::caps) {
			reply = testutils::getline(strm);
			if (cap.compare(reply)) {
				return false;
			}
		}
		return true;
	}

	struct test_git_cmd_handler {
		static void ping(std::ostream& strm)
				{ strm << githlpr::cmd::ping << std::endl; }
		static void capabilities(std::ostream& strm)
				{ strm << githlpr::cmd::caps << std::endl; }
		static void list(std::ostream& strm)
				{ strm << githlpr::cmd::list << std::endl; }
		static void push(std::ostream& strm, const std::vector<std::string>& refspecs)
		{
			if (refspecs.empty()) {
				strm << githlpr::cmd::push << std::endl;
				return;
			}
			for (const auto& refspec : refspecs) {
				strm << githlpr::cmd::push << ' ' << refspec << std::endl;
			}
		}

		static void fetch(std::ostream& strm, const std::vector<std::string>& refspecs)
		{
			if (refspecs.empty()) {
				strm << githlpr::cmd::fetch << std::endl;
				return;
			}
			for (const auto& refspec : refspecs) {
				strm << githlpr::cmd::fetch << ' ' << refspec << std::endl;
			}
		}
	};
}

TEST_CASE("has_valid_git_dir() should return false if GIT_DIR is unset")
{
	CHECK_FALSE(githlpr::has_valid_git_dir_env());
	testutils::setup::set_env("GIT_DIR", ".");
	CHECK(githlpr::has_valid_git_dir_env());
	// ensure env has been reset
	REQUIRE_MESSAGE(not unsetenv("GIT_DIR"),
			("cannot unset env GIT_DIR"));
	REQUIRE_FALSE(githlpr::has_valid_git_dir_env());
}

TEST_SUITE("git_cmd_handler")
{
	TEST_CASE("capabilities")
	{
		std::stringstream strm{};
		githlpr::git_cmd_handler::capabilities(strm);
		CHECK(is_caps_reply(strm));
	}

	TEST_CASE("ping")
	{
		std::stringstream strm{};
		githlpr::git_cmd_handler::ping(strm);
		CHECK(is_ping_reply(strm));
	}

	TEST_CASE("push cmd" * doctest::skip(true))
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should throw on invalid 'push' cmd")
		{
			git_cmd_strm << githlpr::cmd::push << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm), "could not parse dst-ref from push argument");
		}

		SUBCASE("should reply 'ok <dst>' for each 'push' cmd")
		{
			git_cmd_strm << "push refs/heads/master:refs/heads/master" << std::endl;
			git_cmd_strm << "push HEAD:refs/heads/branch" << std::endl;
			githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm);
			CHECK_EQ("ok refs/heads/master", testutils::getline(git_reply_strm));
			testutils::skip_to_blank_or_eof(git_reply_strm);
			CHECK_EQ("ok refs/heads/branch", testutils::getline(git_reply_strm));
			CHECK(testutils::is_last_reply(git_reply_strm));
		}
	}
}

TEST_SUITE("process_git_line_cmds()")
{
	TEST_CASE("line protocol tests")
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};
		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should reply nothing when no cmds were given")
		{
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should reply nothing when only blank lines were given")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should reply nothing to leading blank lines before cmd")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should reply nothing to trailing blank lines after cmd")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should reply nothing to blank lines inbetween cmds")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
			CHECK(testutils::is_empty_line(git_reply_strm));
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
		}

		SUBCASE("should reply to single cmds without termination by blank line")
		{
			// Only replies must be terminated by a blank line,
			// cmds are not terminated by a blank line and should be
			// processed/replied to immediately when received
			// (except for batch cmds: fetch and push which are terminated by a blank line)
			// See next test for batch cmd
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
		}

		SUBCASE("should reply to batch cmds only when terminated by blank line")
		{
			git_cmd_strm << githlpr::cmd::push << " test_data" << std::endl;
			CHECK_THROWS(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm));
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should wait batch cmd execution until terminated by blank line")
		{
			testutils::mt::safe_sstream git_cmds{};
			testutils::mt::safe_sstream git_reply{};
			std::thread git_cmd_processing(githlpr::process_git_line_cmds<test_git_cmd_handler>, std::ref(git_cmds), std::ref(git_reply));
			git_cmds << githlpr::cmd::push << ' ' << 99 << std::endl;
			git_cmds << githlpr::cmd::push << ' ' << 101 << std::endl;
			CHECK(testutils::mt::is_strm_eof_delayed(git_reply));
			git_cmds << std::endl;
			//CHECK(not testutils::mt::is_strm_eof_delayed(git_reply));
			git_cmd_processing.join();
		}

		SUBCASE("should pass entire batch cmds grouped to the handler once terminated by a blank line")
		{
			const std::array<int, 4> args = { 234, 928, 840, 19 };
			for (const int i : args) {
				git_cmd_strm << githlpr::cmd::push << ' ' << i << std::endl;
			}
			git_cmd_strm << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			for (const int i : args) {
				std::stringstream reply_test{};
				reply_test << githlpr::cmd::push << ' ' << i;
				CHECK(testutils::is_reply_equal_to(git_reply_strm, reply_test.str()));
			}
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should throw when batch cmds not terminated by blank line on cmd stream EOF")
		{
			git_cmd_strm << githlpr::cmd::push << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm), "last batch command not terminated");
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should throw when batch cmds not terminated by blank line followed by single type cmd")
		{
			git_cmd_strm << githlpr::cmd::push << std::endl;
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm), "last batch command not terminated");
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should throw when batch cmds not terminated by blank line followed by another batch type cmd")
		{
			git_cmd_strm << githlpr::cmd::push << std::endl;
			git_cmd_strm << githlpr::cmd::fetch << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm), "last batch command not terminated");
			CHECK(testutils::is_last_reply(git_reply_strm));
		}

		SUBCASE("should terminate single reply with a blank line")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			// Discard current reply block until its end
			CHECK(testutils::skip_to_blank_or_eof(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should terminate multiple cmd replies each with a blank line")
		{
			const std::array<std::string_view, 3> cmds = {
				githlpr::cmd::ping,
				githlpr::cmd::ping,
				githlpr::cmd::ping
			};
			for (const std::string_view& cmd : cmds) {
				git_cmd_strm << cmd << std::endl;
			}
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			for (const std::string_view& cmd : cmds) {
				CHECK(testutils::is_reply_equal_to(git_reply_strm, cmd));
				CHECK(testutils::is_empty_line(git_reply_strm));
			}
		}

		SUBCASE("should reply to cmds in order they were received")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			git_cmd_strm << githlpr::cmd::caps << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
			testutils::skip_to_blank_or_eof(git_reply_strm); // skip over blank line
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::caps));
		}

		SUBCASE("should not repeat past replies in new cmds")
		{
			const std::array<std::string_view, 3> cmds = {
				githlpr::cmd::ping,
				githlpr::cmd::caps,
				githlpr::cmd::list,
			};
			for (const std::string_view& cmd : cmds) {
				git_cmd_strm << cmd << std::endl;
			}
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			for (const std::string_view& cmd : cmds) {
				CHECK(testutils::is_reply_equal_to(git_reply_strm, cmd));
				testutils::is_empty_line(git_reply_strm);
			}
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}
	}

	TEST_CASE("line processing tests")
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should throw on unknown command")
		{
			git_cmd_strm << "foo bar" << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm), "unknown command: foo");
		}

		SUBCASE("should throw on unknown command even when other cmds were valid")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			git_cmd_strm << "foo bar" << std::endl;
			git_cmd_strm << githlpr::cmd::caps << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm), "unknown command: foo");
		}

		SUBCASE("should call ping cmd handler")
		{
			git_cmd_strm << githlpr::cmd::ping << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::ping));
		}

		SUBCASE("should call capabilities cmd handler")
		{
			git_cmd_strm << githlpr::cmd::caps << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::caps));
		}

		SUBCASE("should call list cmd handler")
		{
			git_cmd_strm << githlpr::cmd::list << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::list));
		}

		SUBCASE("should call push cmd handler")
		{
			git_cmd_strm << githlpr::cmd::push << std::endl;
			git_cmd_strm << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::push));
		}

		SUBCASE("should call fetch cmd handler")
		{
			git_cmd_strm << githlpr::cmd::fetch << std::endl;
			git_cmd_strm << std::endl;
			githlpr::process_git_line_cmds<test_git_cmd_handler>(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_reply_equal_to(git_reply_strm, githlpr::cmd::fetch));
		}
	}

	TEST_CASE("list cmd" * doctest::skip(true))
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should reply dummy refs on 'list for-push' cmd")
		{
			git_cmd_strm << "list for-push" << std::endl;
			githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm);
			CHECK_EQ("2a569a9e9e5a0d8e4ce829bbdd84904633024f86 refs/heads/master", testutils::getline(git_reply_strm));
			CHECK(testutils::is_last_reply(git_reply_strm));
		}
	}

	/* Leaving fetch out as first we restructure the git cmd handling logic
	TEST_CASE("fetch cmd")
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should wait until single fetch cmd is terminated by blank line")
		{
			testutils::safe_sstream cmd{}, reply{};
			// TODO
		}

		SUBCASE("should wait until fetch cmd block is terminated by blank line")
		{
			testutils::safe_sstream cmd{}, reply{};
			// TODO
		}

		SUBCASE("should throw on parameterless fetch cmd")
		{
			git_cmd_strm << githlpr::cmd::fetch << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm), "could not parse fetch parameters");
		}

		SUBCASE("should throw on unterminated fetch cmd")
		{
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_sha1 << ' ' << test_ref << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm), "fetch command was not terminated");
		}

		SUBCASE("should throw on invalid fetch cmd (missing ref)")
		{
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_sha1 << std::endl;
			git_cmd_strm << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm), "could not parse fetch parameters");
		}

		SUBCASE("should throw on invalid fetch cmd (missing sha1 hash)")
		{
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_ref << std::endl;
			git_cmd_strm << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm), "could not parse fetch parameters");
		}

		SUBCASE("should reply blank line on single fetch cmd")
		{
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_sha1 << ' ' << test_ref << std::endl;
			git_cmd_strm << std::endl;
			githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm);
			CHECK(testutils::getline(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should reply single blank line at the end of fetch cmd block")
		{
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_sha1 << ' ' << test_ref << std::endl;
			git_cmd_strm << githlpr::cmd::fetch << ' ' << test_sha1 << ' ' << test_ref << std::endl;
			git_cmd_strm << std::endl;
			githlpr::process_git_line_cmds(git_cmd_strm, git_reply_strm);
			CHECK(testutils::getline(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}
	}
	*/
}
