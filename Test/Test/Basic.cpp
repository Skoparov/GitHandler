#include "gtest/gtest.h"
#include<boost/filesystem.hpp>

#include "GitHandler.h"
#include "TestArgs.h"

extern TestArgs testArgs;
using namespace git_handler;

class GitHandlerTest : public testing::Test
{
protected:
    virtual void SetUp(){}
    virtual void TearDown(){}

protected:
    GitHandler mHandler;
};

TEST_F( GitHandlerTest, OpenLocal )
{
    auto repo = base::Repo{};
    ASSERT_FALSE( repo.openLocal( "" ) ) << "Empty opening local repo didn't fail";
    ASSERT_TRUE( repo.openLocal( testArgs.localRepoPath ) ) << "Opening local repo failed";
}

TEST_F( GitHandlerTest, Clone )
{
    auto repo = base::Repo{};
    ASSERT_FALSE( repo.clone( "", "" ) ) << "Empty clone didn't fail";
    bool cloneResult = repo.clone( testArgs.remoteRepoUrl, testArgs.remoteRepoLocalPath );

    if( cloneResult )
    {
        ASSERT_TRUE( repo.isValid() );
        ASSERT_TRUE( repo.fetch() );

        base::Repo::BranchStorage branches;
        ASSERT_TRUE( repo.getBranches( branches ) );

        for( const auto& branch : branches )
        {
            auto name = branch.first;
            ASSERT_TRUE( branch.second != nullptr );
            ASSERT_TRUE( repo.getBranch( name ) != nullptr );
        }
    }

    boost::filesystem::remove_all( testArgs.remoteRepoLocalPath );
    ASSERT_TRUE( cloneResult ) << "Cloning remote repo failed";
}

// GitHandler h;

// string path = "/home/skoparov/Repo/dir";
// //string path = "D:\\Repo\\libgit2";


// RepoPtr repo = std::make_shared<Repo>();
// bool result = repo->openLocal(path);
// h.addRepo(repo, "Skoparov", "Q3e5T7u9");

// //bool a = h.update();
// //printf("Fetch %s\n", a ? "successful" : "failed");
// //printNew(h);

// auto repoPtr = h.getRepo(path);
// if (repoPtr != nullptr)
// {
//     BranchStorage locals;
//     if (repoPtr->getBranches(locals, true))
//     {
//         Aux::printBranches(locals);
//         for (auto branch : locals){
//             Aux::printBranchCommits(branch.second);
//         }
//     }
// }

