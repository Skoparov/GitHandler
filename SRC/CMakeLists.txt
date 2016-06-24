cmake_minimum_required(VERSION 2.8)

add_definitions("-std=c++11")

project (${GIT_HANDLER_PROJECT_NAME})
message(STATUS ${PROJECT_NAME})

set (HEADERS 	GitDeleters.h	
                GitItem.h
                GitItemFactory.h 						
				GitBaseClasses.h
                GitHandler.h
)		
				
set (SOURCES 	GitBaseClasses.cpp 
                GitHandler.cpp
)

#Create shared lib
add_library(	${PROJECT_NAME} SHARED 
				${HEADERS} 
				${SOURCES}
)

#Libs				
target_link_libraries(	${PROJECT_NAME} 
						${Boost_LIBRARIES}
						${LIBGIT2_LIB}
)

#set_target_properties("${PROJECT_NAME}"  PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")