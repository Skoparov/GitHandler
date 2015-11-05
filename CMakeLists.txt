cmake_minimum_required(VERSION 2.8)

add_definitions("-std=c++11")

project ("GitHandler")
message(STATUS ${PROJECT_NAME})

SET(CMAKE_BUILD_TYPE Debug)

set (CMAKE_FIND_ROOT_PATH ${ENV_PATH})
set (CMAKE_PREFIX_PATH ${ENV_PATH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/)

find_package(Boost 1.55.0 REQUIRED system thread date_time unit_test_framework)

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Cannot find boost")
else()
  message(STATUS "Setting up BOOST")
  message(STATUS " Includes - ${Boost_INCLUDE_DIRS}")
  message(STATUS " Library  - ${Boost_LIBRARY_DIRS}") 
endif(NOT Boost_FOUND)	

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/SRC)
include_directories(${SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/)

set (HEADERS 	${SOURCE_DIR}/GitItemFactory.h 
				${SOURCE_DIR}/Deleters.h
                ${SOURCE_DIR}/githandler.h
				${SOURCE_DIR}/include/git2.h)		
				
set (SOURCES 	${SOURCE_DIR}/githandler.cpp 
				${SOURCE_DIR}/main.cpp)	

#Boost				
link_directories(${Boost_LIBRARY_DIRS})	
include_directories(${Boost_INCLUDE_DIRS})

#libGit
include_directories(${SOURCE_DIR}/SRC/include/)

link_directories (${PROJECT_SOURCE_DIR}/LIB/)		
add_executable(${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} git2)
set_target_properties("${PROJECT_NAME}"  PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
