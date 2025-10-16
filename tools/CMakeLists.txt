add_executable(git-remote-interactive-proto git-remote-interactive-proto.cpp)
add_executable(interactive-proto-runner interactive-proto-runner.cpp)
target_include_directories(interactive-proto-runner PRIVATE ../test)
target_link_libraries(interactive-proto-runner PRIVATE doctest::doctest)
add_executable(interactive-proto-frontend interactive-proto-frontend.cpp)

add_custom_target(tools-run-interactive-proto
	COMMAND BINARY_SEARCH_PATH=$<TARGET_FILE_DIR:git-remote-interactive-proto> $<TARGET_FILE:interactive-proto-runner>
	DEPENDS interactive-proto-runner interactive-proto-frontend git-remote-interactive-proto
	COMMENT "Running interactive git-remote-helper backend..."
	USES_TERMINAL)
