#ifndef GITBASECLASSES_H
#define GITBASECLASSES_H

#include <set>
#include <string>

#include "GitItemFactory.h"

namespace git_handler
{

namespace base
{

// Item aliases
using GitRef     = item::GitItem< git_reference >;
using GitRepo    = item::GitItem< git_repository >;
using GitRemote  = item::GitItem< git_remote >;
using GitCommit  = item::GitItem< git_commit >;
using GitStrArr  = item::GitItem< git_strarray >;
using GitRevWalk = item::GitItem< git_revwalk >;

class Repo;

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Commit : public std::enable_shared_from_this< Commit >
{
public:
    explicit Commit( std::unique_ptr< GitCommit >&& commit = nullptr );

    git_oid id() const noexcept;
    git_time time() const noexcept;
    std::string author() const noexcept;
    std::string message() const noexcept;
    bool isValid() const noexcept;

private:	
    std::unique_ptr< GitCommit > mCommit;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Branch : public std::enable_shared_from_this< Branch >
{
    friend class Repo;

public:
    using CommitId      = std::pair< git_time_t, std::string >;
    using CommitStorage = std::map< CommitId, std::unique_ptr< Commit > >;

public:
    Branch( std::unique_ptr< GitRef >&& branchRef = nullptr,
            const bool isRemote = false,
            const std::shared_ptr< Repo > parentRepo = nullptr );

    void addCommit( std::unique_ptr< Commit >&& commit );
    void clearCommits() noexcept;
	
    std::string name() const noexcept;
    const CommitStorage& commits() const noexcept;

    bool isRemote() const noexcept;
    bool isValid() const noexcept;
	
private:
    CommitStorage mCommits;
    std::unique_ptr< GitRef > mBranchRef;
    bool mIsRemote;
    std::weak_ptr< Repo > mParentRepo;
};

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Repo                  //////////////////////
////////////////////////////////////////////////////////////////////////////////

class Repo : public std::enable_shared_from_this< Repo >
{
public:
    using BranchStorage = std::map< std::string, std::unique_ptr< Branch > >;
    using Remotes = std::map< std::string, std::unique_ptr< GitRemote > >;

private:
    using RemotesList   = std::set< std::string >;

public:
    Repo( const std::string& repoPath = {},
          std::unique_ptr< GitRepo >&& repo = nullptr );
	
    // repo operations
    bool openLocal( const std::string& path );
    bool fetch( const git_fetch_options& fetch_opts = GIT_FETCH_OPTIONS_INIT );
    bool clone( const std::string& url, const std::string& path, const git_clone_options& cloneOpts = GIT_CLONE_OPTIONS_INIT );
    void closeRepo() noexcept;

    // getters
    bool isValid() const noexcept;
    std::string path() const noexcept;
    std::unique_ptr< Branch > getBranch( const std::string& refName );
    bool getBranches( BranchStorage& branchStor, const bool getRemotes = false );

private:
    bool readRemotesList( RemotesList& remotesList );
    bool readBranchCommits( Branch* branch);
    bool updateRemotes(const git_fetch_options& fetch_opts);

private:
    std::string mLocalPath;
    std::unique_ptr< GitRepo > mGitRepo;
    Remotes mRemotes;
};

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Aux                   //////////////////////
////////////////////////////////////////////////////////////////////////////////

class Aux
{
public:
    static std::unique_ptr< GitStrArr > createStrArr();
    static std::string                  getCommitMessageStr( const Commit* commit );
    static std::unique_ptr< GitRef >    getReference( const std::string& refName, const GitRepo* repo );
    static std::unique_ptr< GitCommit > readCommit(const GitRepo* repo, const git_oid* head );
    static std::unique_ptr< GitStrArr > getRepoRefList( const GitRepo* repo );
    static std::string                  getBranchName( const std::string& fullBranchName );

    static void printBranches( const Repo::BranchStorage& storage );
    static void printCommit( const Commit* commit );
    static void printBranchCommits( const Branch* branch );
};

}//base

}//git_handler

#endif // GITBASECLASSES_H
