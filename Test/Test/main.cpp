#include "gtest/gtest.h"
#include "TestArgs.h"

TestArgs testArgs;

enum Arg
{
    ARG_LOCAL_REPO_PATH = 1,
    ARG_REMOTE_REPO_URL,
    ARG_REMOTE_REPO_LOCAL_PATH,

    ARG_TOTAL_COUNT
};

int main(int argc, char **argv)
{    
    ::testing::InitGoogleTest(&argc, argv);
    if( argc != ARG_TOTAL_COUNT )
    {
        std::cout<<"Wrong number of arguments.\n"
                   "1 - should be the path to a local test repo\n";
                   "1 - should be the url to a remote test repo\n";
        return 0;
    }

    testArgs.localRepoPath = argv[ ARG_LOCAL_REPO_PATH ];
    testArgs.remoteRepoUrl = argv[ ARG_REMOTE_REPO_URL ];
    testArgs.remoteRepoLocalPath = argv[ ARG_REMOTE_REPO_LOCAL_PATH ];

    return RUN_ALL_TESTS();
}
