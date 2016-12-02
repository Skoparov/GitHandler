cmake_minimum_required(VERSION 2.8)

add_definitions("-std=c++14 ")

project( ${GIT_HANDLER_TESTS_PROJECT_NAME} )
message(STATUS ${PROJECT_NAME})

find_package(Threads REQUIRED)

add_subdirectory(Test)
include_directories (Test)
