#ifndef GITHANDLER_H
#define GITHANDLER_H

#include<vector>

#include "GitBaseClasses.h"

namespace git_handler
{

//////////////////////////////////////////////////////////////////////////////
///////////////                GitHandler               //////////////////////
//////////////////////////////////////////////////////////////////////////////

class git_handler
{
public:
    using new_braches = std::map< std::string, std::vector< base::branch_wrapper* > >;
    using new_commits = std::map< std::pair< std::string, std::string >, std::vector< base::commit_wrapper* > > ;
    using credentials = std::map< std::string, std::pair< std::string, std::string > >;
    using repos = std::map< std::string, std::unique_ptr< base::repo_wrapper > >;

public:
    git_handler();
    git_handler( const git_handler& ) = delete;
    git_handler( git_handler&& ) = delete;
    git_handler& operator=( const git_handler& ) = delete;
    git_handler& operator=( git_handler&& ) = delete;
    ~git_handler();

    bool add_repo( std::unique_ptr< base::repo_wrapper >&& repo, const std::string& username, const std::string& pass );
    void update() noexcept;
    void clear() noexcept;
		
    base::repo_wrapper* getRepo(const std::string& path) const noexcept;
    const repos& get_repos() const noexcept;

//    NewBranchStorage newBranches();
//    NewCommitStorage newCommits();

private:
    bool register_git_items();

    //callbacks with params determined by the lib
    static int progress_cb(const char *str, int len, void *data);
    static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);
    static int cred_acquire_cb(git_cred **out, const char* url, const char* username_from_url, unsigned int allowed_typed, void* data);

private:
    repos m_repos;

    static credentials mCredentials;
    static base::repo_wrapper* mCurrentRepo;

    static new_braches m_new_branches;
    static new_commits m_new_commits;
};

} //git_handler

#endif // GITHANDLER_H
