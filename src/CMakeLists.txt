add_executable(git-remote-rclone main.cpp)
target_link_libraries(git-remote-rclone PRIVATE githlpr utils)

add_library(githlpr githlpr.cpp)
target_include_directories(githlpr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(githlpr PRIVATE utils)

add_library(utils utils.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
