cmake_minimum_required(VERSION 2.8)

project ("General")
message(STATUS ${PROJECT_NAME})

SET(CMAKE_BUILD_TYPE Debug)

################################
# Directories
################################

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/${CMAKE_BUILD_TYPE})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/LIB/${CMAKE_BUILD_TYPE})

################################
# Boost
################################

find_package(Boost 1.55.0 REQUIRED system thread date_time unit_test_framework)

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Cannot find boost")
else()
  	message(STATUS "Setting up BOOST")
	message(STATUS "Boost includes : " ${Boost_INCLUDE_DIRS})
	message(STATUS "Boost libs : " ${Boost_LIBRARY_DIRS})
endif(NOT Boost_FOUND)			

################################
# LibGit2
################################

include(ExternalProject)

# Download and install LibGit2
ExternalProject_Add(
	git2
	GIT_REPOSITORY https://github.com/libgit2/libgit2.git
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libgit2
	#INSTALL_DIR ${PROJECT_SOURCE_DIR}/libgit2	
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)

if (WIN32)
	message(STATUS "WIN32")
	set(LIBGIT2_NAME libgit2.dll)
elseif(UNIX)    
	message(STATUS "UNIX")
	set(LIBGIT2_NAME libgit2.so)
else()
	message(FATAL_ERROR "Cannot determine OS type")	
endif (WIN32)

ExternalProject_Get_Property(git2 install_dir)
message(STATUS "INSTALL_DIR")
message(STATUS ${install_dir})

set(LIBGIT2_LIBRARY_DIRS ${install_dir}/lib)
set(LIBGIT2_INCLUDE_DIRS ${install_dir}/include)
set(LIBGIT2_LIB ${LIBGIT2_LIBRARY_DIRS}/${LIBGIT2_NAME})

################################
# Includes and links
################################

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LIBGIT2_INCLUDE_DIRS})

include_directories("./")
include_directories("./SRC")
include_directories("./Test")
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(SRC)