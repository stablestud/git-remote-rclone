#include <filesystem>

#include "testutils.hpp"

int main(int argc, char* argv[])
{
	testutils::setup::setup_workdir(std::filesystem::absolute(argv[0]));
	testutils::git::git_repo repo = testutils::git::init_repo(testutils::WORKDIR / "repo");
	testutils::git::add_remote(repo, "interactive-proto://repo"); // make git call git-remote-interactive-proto on push
	testutils::git::append_test_data(repo);
	testutils::git::add_all(repo);
	testutils::git::commit(repo);
	testutils::git::push(repo); // Make git to call git-remote-interactive-proto
}
