include(FetchContent)
FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG v2.4.12
		GIT_SHALLOW True)
FetchContent_MakeAvailable(doctest)

# test_integration
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration PRIVATE doctest::doctest)
add_dependencies(test_integration git-remote-rclone)
add_test(NAME test_integration COMMAND $<TARGET_FILE:test_integration>)
set_tests_properties(test_integration PROPERTIES ENVIRONMENT BINARY_SEARCH_PATH=$<TARGET_FILE_DIR:git-remote-rclone>)

# test_githlpr
add_executable(test_githlpr test_githlpr.cpp)
target_link_libraries(test_githlpr PRIVATE doctest::doctest githlpr)
add_test(NAME test_githlpr COMMAND $<TARGET_FILE:test_githlpr>)
