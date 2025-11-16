add_executable(git-remote-interactive-proto git-remote-interactive-proto.cpp)
add_executable(git-remote-interactive-proto-attach git-remote-interactive-proto-attach.cpp)
add_executable(git-remote-interactive-proto-runner git-remote-interactive-proto-runner.cpp)
target_include_directories(git-remote-interactive-proto-runner PRIVATE ../tests)

add_custom_target(run-git-remote-interactive-proto
		COMMAND BINARY_SEARCH_PATH=$<TARGET_FILE_DIR:git-remote-interactive-proto> $<TARGET_FILE:git-remote-interactive-proto-runner>
		DEPENDS git-remote-interactive-proto-runner git-remote-interactive-proto-attach git-remote-interactive-proto
		COMMENT "Running interactive git-remote-helper protocol inspector..."
		USES_TERMINAL)
