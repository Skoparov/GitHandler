cmake_minimum_required (VERSION 2.6)

project ("GitUpdater")
message(STATUS "Git Updater")

set (QT_DIR $ENV{QTDIR64})
find_package(Qt4 4.8.4 REQUIRED QtCore QtGui QtNetwork)
include (${QT_USE_FILE})
set (CMAKE_AUTOMOC ON)

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/SRC)
include_directories(${SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

#libGit
include_directories(${SOURCE_DIR}/SRC/include/)
link_directories (${PROJECT_SOURCE_DIR}/LIB/)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/)

set (HEADERS 	${SOURCE_DIR}/gitupdater.h 
				${SOURCE_DIR}/include/git2.h)		
				
set (SOURCES 	${SOURCE_DIR}/gitupdater.cpp 
				${SOURCE_DIR}/main.cpp)	 
				
set (QRC 		${SOURCE_DIR}/gitupdater.qrc)
			 
#resources
qt4_wrap_ui (UI ${SOURCE_DIR}/gitupdater.ui)
qt4_add_resources (RES ${SOURCE_DIR}/gitupdater.qrc)
set (GEN_FILES ${UI} ${RES})

add_executable(			${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES} ${GEN_FILES} ${QRC})
set_target_properties("${PROJECT_NAME}"  PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
target_link_libraries(	${PROJECT_NAME}  ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY} git2)