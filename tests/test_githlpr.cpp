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

const int cap_reply_line_count = 1;

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

		SUBCASE("should reply nothing and return true when cmd not terminated by blank line")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			CHECK(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm));
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should reply nothing and return true when no cmds were given")
		{
			CHECK(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm));
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should terminate replies with a blank line and EOF")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			// Discard current reply block until its end
			CHECK(testutils::skip_to_blank_or_eof(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
		}

		SUBCASE("should ignore blank lines which are not terminating cmds")
		{
			git_cmd_strm << std::endl << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;
			git_cmd_strm << std::endl << std::endl;

			CHECK_FALSE(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm));

			CHECK_EQ(1, testutils::get_count_empty_lines(git_reply_strm));
		}

		SUBCASE("should only reply (start processing cmds) when cmd has been terminated by a blank line")
		{
			std::stringstream git_cmd_strm = testutils::safe_sstream{};
			std::stringstream git_reply_strm = testutils::safe_sstream{};

			std::future<bool> f = std::async(std::launch::async, &githlpr::process_git_cmds, std::ref(git_cmd_strm), std::ref(git_reply_strm));

			git_cmd_strm << githlpr::cmds::ping << std::endl;

			{ 
				using namespace std::chrono_literals;
			       	CHECK(testutils::is_strm_eof_delayed(git_reply_strm, 200ms));
			}

			f.wait(); // join with thread
		}

		SUBCASE("should reply coherently to single coherent command block (cmd as block -> reply as block, no terminating blank line between replies)")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			CHECK_EQ(1, testutils::get_count_empty_lines(git_reply_strm));		
		}

		SUBCASE("should reply to each cmd block as reply block")
		{
			// Block 1
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;
			// Block 2
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			// Test block 1
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(testutils::getline(git_reply_strm).empty());
			// Test block 2
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(testutils::getline(git_reply_strm).empty());
		}

		SUBCASE("should reply to cmds in order they were received")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl;
			git_cmd_strm << githlpr::cmds::caps << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			CHECK(is_ping_reply(git_reply_strm));
			testutils::getline(git_reply_strm); // skip over blank line
			CHECK_EQ(githlpr::replies::capabilities, testutils::getline(git_reply_strm));
		}

		SUBCASE("should reply to cmds in order they were received (inside blocks)")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << githlpr::cmds::caps << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			CHECK(is_ping_reply(git_reply_strm));
			CHECK_EQ(githlpr::replies::capabilities, testutils::getline(git_reply_strm));
		}
	}

	TEST_CASE("data processing tests")
	{
		std::stringstream git_cmd_strm{};
		std::stringstream git_reply_strm{};

		CHECK(git_cmd_strm.str().empty());
		CHECK(git_reply_strm.str().empty());

		SUBCASE("should return true on unknown command")
		{
			git_cmd_strm << "foo bar" << std::endl;
			git_cmd_strm << std::endl;

			CHECK(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm));
		}

		SUBCASE("should return false on valid command")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl;

			CHECK_FALSE(githlpr::process_git_cmds(git_cmd_strm, git_reply_strm));
		}

		SUBCASE("should reply pong on ping cmd")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			CHECK(is_ping_reply(git_reply_strm));
		}

		SUBCASE("should reply its capabilities on capabilities cmd")
		{
			git_cmd_strm << githlpr::cmds::caps << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			CHECK_EQ(githlpr::replies::capabilities, testutils::getline(git_reply_strm));
			// No more capabilities should be replied
			CHECK(testutils::getline(git_reply_strm).empty());
		}

		SUBCASE("should reply 'ok <dst>' for each push cmd")
		{
			git_cmd_strm << "push refs/heads/master:refs/heads/master" << std::endl;
			git_cmd_strm << "push HEAD:refs/heads/branch" << std::endl;
			git_cmd_strm << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);

			CHECK_EQ("ok refs/heads/master", testutils::getline(git_reply_strm));
			CHECK_EQ("ok refs/heads/branch", testutils::getline(git_reply_strm));
			// No more push acks should be replied
			CHECK(testutils::getline(git_reply_strm).empty());
		}

		SUBCASE("should not repeat past replies in later replies")
		{
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;
			git_cmd_strm << githlpr::cmds::ping << std::endl;
			git_cmd_strm << std::endl << std::endl;

			githlpr::process_git_cmds(git_cmd_strm, git_reply_strm);
			std::istringstream copy(std::string(git_reply_strm.str()));

			CHECK(is_ping_reply(git_reply_strm));
			CHECK(testutils::getline(git_reply_strm).empty());
			CHECK(is_ping_reply(git_reply_strm));
			CHECK(testutils::getline(git_reply_strm).empty());
			CHECK(testutils::is_strm_eof(git_reply_strm));
			CHECK_EQ(2, testutils::get_count_empty_lines(copy));
		}
	}
}
