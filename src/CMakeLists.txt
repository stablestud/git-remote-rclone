add_executable(git-remote-rclone main.cpp)
target_link_libraries(git-remote-rclone PRIVATE githlpr)
target_link_options(git-remote-rclone PRIVATE -static)

add_library(githlpr STATIC githlpr.cpp)
target_include_directories(githlpr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
