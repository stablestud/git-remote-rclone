#ifndef DOCTESTUTILS_HPP
#define DOCTESTUTILS_HPP

#include <filesystem>
#include <string>

#include <doctest/doctest.h>

#include "testutils.hpp"

#define SETUP_TEST(x) int main(int argc, char **argv) { return doctestutils::init(argc, argv, (x)); }
#define SETUP_TEST_CASE(x) (testutils::setup::setup_sub_workdir((x)))

namespace doctestutils
{
        int init(const int argc, const char *const *const argv, const std::string test_name) {
                doctest::Context context;
                try {
                        if (nullptr == argv[0]) {
                                throw std::runtime_error("argv[0] is empty");
                        }
                        const std::filesystem::path self(argv[0]);
                        context.applyCommandLine(argc, argv);
                        testutils::setup::setup_workdir(self.parent_path() / test_name);
                } catch(const std::runtime_error& err) {
                        std::cerr << test_name << ": failed to setup test: " << err.what() << std::endl;
                        std::exit(EXIT_FAILURE);
                }
                return context.run();
        }
}

#endif /* DOCTESTUTILS_HPP */
