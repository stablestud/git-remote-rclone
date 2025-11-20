#include <chrono>
#include <future>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <cstdlib>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "testutils.hpp"

#include "githlpr.hpp"

static bool is_ping_reply(std::istream& strm)
{
	return githlpr::replies::ping_reply == testutils::getline(strm);
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

TEST_SUITE("process_git_cmds()")
{
	TEST_CASE("line protocol tests") {
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should reply nothing when no cmds were given")
		{
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should ignore blank cmd lines and reply nothing")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should ignore blank cmd lines, reply to cmd")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << std::endl;
			}
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(is_ping_reply(git_reply_strm));
		}

		SUBCASE("should reply to cmds without terminating blank lines")
		{
			// Only replies must be terminated by a blank line,
			// cmds are not terminated by a blank line and should be
			// processed/replied to immediately when received
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(is_ping_reply(git_reply_strm));
		}

		SUBCASE("should terminate single reply with a blank line")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			// Discard current reply block until its end
			CHECK(testutils::skip_to_blank_or_eof(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should terminate each reply with a blank line")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << githlpr::cmds::ping << std::endl;
				git_cmd_strm << githlpr::cmds::ping << std::endl;
			}
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			for (int i{}; i <= 3; i++) {
				CHECK(is_ping_reply(git_reply_strm));
				CHECK(testutils::getline(git_reply_strm).empty());
			}
		}

		SUBCASE("should reply to cmds in order they were received")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::caps << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(is_ping_reply(git_reply_strm));
			testutils::skip_to_blank_or_eof(git_reply_strm); // skip over blank line
			CHECK_EQ(githlpr::replies::capabilities, testutils::getline(git_reply_strm));
		}

		SUBCASE("should not repeat past cmds in new cmds")
		{
			for (int i{}; i <= 3; i++) {
				git_cmd_strm << githlpr::cmds::ping << std::endl;
			}
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			for (int i{}; i <= 3; i++) {
				CHECK(is_ping_reply(git_reply_strm));
				testutils::skip_to_blank_or_eof(git_reply_strm); // skip over blank line
			}
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}
	}

	TEST_CASE("data processing tests")
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		REQUIRE(git_cmd_strm.str().empty());
		REQUIRE(git_reply_strm.str().empty());

		SUBCASE("should throw on unknown command")
		{
			git_cmd_strm << "foo bar" << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm), "unknown command: foo bar");
		}

		SUBCASE("should throw on unknown command even when other cmds were valid")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << "foo bar" << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm), "unknown command: foo bar");
		}

		SUBCASE("should reply 'pong' on ping cmd")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK(is_ping_reply(git_reply_strm));
		}

		SUBCASE("should reply its capabilities on capabilities cmd")
		{
			git_cmd_strm << githlpr::cmds::caps << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK_EQ(githlpr::replies::capabilities, testutils::getline(git_reply_strm));
		}

		SUBCASE("should throw on invalid push cmd")
		{
			git_cmd_strm << githlpr::cmds::push << std::endl;
			CHECK_THROWS_WITH(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm), "could not parse dst-ref from push argument");
		}

		SUBCASE("should reply 'ok <dst>' for each push cmd")
		{
			git_cmd_strm << "push refs/heads/master:refs/heads/master" << std::endl;
			git_cmd_strm << "push HEAD:refs/heads/branch" << std::endl;
			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK_EQ("ok refs/heads/master", testutils::getline(git_reply_strm));
			testutils::skip_to_blank_or_eof(git_reply_strm);
			CHECK_EQ("ok refs/heads/branch", testutils::getline(git_reply_strm));
		}
	}
}
