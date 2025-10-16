#include <iostream>

#include <cstdlib>

#include "utils.hpp"
#include "githlpr.hpp"

int main()
{
	if (not githlpr::has_valid_git_dir_env()) {
		std::cerr << "GIT_DIR is not set" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	DEBUG_WAIT;
	return githlpr::process_git_cmds(std::cin, std::cout);
}
