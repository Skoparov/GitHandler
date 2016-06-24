cmake_minimum_required(VERSION 2.8)

add_definitions("-std=c++11")

project ("GitHandler")
message(STATUS ${PROJECT_NAME})

set (HEADERS 	GitDeleters.h	
                GitItem.h
                GitItemFactory.h 						
				GitBaseClasses.h
                GitHandler.h
                )		
				
set (SOURCES 	GitBaseClasses.cpp 
                GitHandler.cpp                
				main.cpp)

add_executable(${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES})

link_directories(${LIBGIT2_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${LIBGIT2_LIB})

#set_target_properties("${PROJECT_NAME}"  PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")