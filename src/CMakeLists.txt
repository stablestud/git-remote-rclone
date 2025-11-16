add_executable(git-remote-rclone main.cpp)
target_link_libraries(git-remote-rclone PRIVATE githlpr)

add_library(githlpr githlpr.cpp)
target_include_directories(githlpr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
