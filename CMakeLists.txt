cmake_minimum_required(VERSION 3.28)

project(git-remote-rclone VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(DEBUG "build with debugging flags / define DEBUG" OFF)

get_filename_component(BUILD_DIR "${CMAKE_BINARY_DIR}" NAME)

if(DEBUG)
	add_compile_definitions(DEBUG)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
	message(STATUS "Configuring ${BUILD_DIR} as DEBUG build")
else()
	message(STATUS "Configuring ${BUILD_DIR} as RELEASE build")
endif()

add_subdirectory(src)
if(DEBUG)
	add_subdirectory(test)
	add_subdirectory(tools)
endif()

enable_testing()
