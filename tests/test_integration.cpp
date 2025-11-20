#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT

#include "doctestutils.hpp"
#include "testutils.hpp"

namespace git = testutils::git;

SETUP_TEST("test_integration");

TEST_CASE("walking_skeleton")
{
	const std::filesystem::path test_case_dir = SETUP_TEST_CASE("walking_skeleton");
	git::git_repo init_repo = git::init_repo(test_case_dir / "init_repo");
	git::add_remote(init_repo);
	git::append_test_data(init_repo);
	git::add_all(init_repo);
	git::commit(init_repo);
	CHECK(git::push(init_repo));
}
