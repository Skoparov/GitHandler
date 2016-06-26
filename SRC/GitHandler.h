#ifndef GITHANDLER_H
#define GITHANDLER_H

#include<vector>

#include "GitBaseClasses.h"

namespace git_handler
{

//////////////////////////////////////////////////////////////////////////////
///////////////                GitHandler               //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitHandler
{
public:
    using NewBranchStorage = std::map< std::string, std::vector< base::Branch* > >;
    using NewCommitStorage = std::map< std::pair< std::string, std::string >, std::vector< base::Commit* > > ;
    using Credentials      = std::map< std::string, std::pair< std::string, std::string > >;
    using RepoStorage      = std::map< std::string, std::unique_ptr< base::Repo > >;


    GitHandler();
    GitHandler( const GitHandler& ) = delete;
    GitHandler( GitHandler&& ) = delete;
    GitHandler& operator=( const GitHandler& ) = delete;
    GitHandler& operator=( GitHandler&& ) = delete;
    ~GitHandler();

    bool addRepo( std::unique_ptr< base::Repo >&& repo, const std::string& username, const std::string& pass );
    bool update() noexcept;
    void clear() noexcept;
		
    base::Repo* getRepo(const std::string& path) const noexcept;
    const RepoStorage& getRepos() const noexcept;

//    NewBranchStorage newBranches();
//    NewCommitStorage newCommits();

private:
    bool registerGitItemTypes();

    //callbacks with params determined by the lib
    static int progress_cb(const char *str, int len, void *data);
    static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);
    static int cred_acquire_cb(git_cred **out, const char* url, const char* username_from_url, unsigned int allowed_typed, void* data);

private:
    RepoStorage mRepos;

    static Credentials mCredentials;
    static base::Repo* mCurrentRepo;

    static NewBranchStorage mNewBranches;
    static NewCommitStorage mNewCommits;
};

} //git_handler

#endif // GITHANDLER_H
