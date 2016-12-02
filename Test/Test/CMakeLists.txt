#Get all test files
file(GLOB CPPS *.cpp *.h)

add_executable(	${PROJECT_NAME} ${CPPS}	)

target_link_libraries( ${PROJECT_NAME}
                       ${LIBGIT2_LIB}
                       ${GIT_HANDLER_PROJECT_NAME}
                       gtest
                       )

