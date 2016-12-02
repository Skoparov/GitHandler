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
using git_item_ref = item::git_item< git_reference >;
using git_item_repo = item::git_item< git_repository >;
using git_item_remote = item::git_item< git_remote >;
using git_item_commit = item::git_item< git_commit >;
using git_item_str_arr = item::git_item< git_strarray >;
using git_item_rev_walk = item::git_item< git_revwalk >;

class repo_wrapper;

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class commit_wrapper : public std::enable_shared_from_this< commit_wrapper >
{
public:
    explicit commit_wrapper( std::unique_ptr< git_item_commit >&& commit = nullptr );

    git_oid id() const noexcept;
    git_time time() const noexcept;
    std::string author() const noexcept;
    std::string message() const noexcept;
    bool isValid() const noexcept;

private:	
    std::unique_ptr< git_item_commit > m_commit;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class branch_wrapper : public std::enable_shared_from_this< branch_wrapper >
{
    friend class repo_wrapper;

public:
    using commit_id = std::pair< git_time_t, std::string >;
    using commit_storage = std::map< commit_id, std::unique_ptr< commit_wrapper > >;

public:
    branch_wrapper( std::unique_ptr< git_item_ref >&& branch_ref = nullptr,
            const bool is_remote = false,
            const std::shared_ptr< repo_wrapper > parent_repo = nullptr );

    void add_commit( std::unique_ptr< commit_wrapper >&& commit_wrapper );
    void clear_commits() noexcept;
	
    std::string name() const noexcept;
    const commit_storage& commits() const noexcept;

    bool is_remote() const noexcept;
    bool is_valid() const noexcept;
	
private:
    bool m_is_remote;
    commit_storage m_commits;
    std::weak_ptr< repo_wrapper > m_parent_repo;
    std::unique_ptr< git_item_ref > m_branch_ref;

};

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Repo                  //////////////////////
////////////////////////////////////////////////////////////////////////////////

class repo_wrapper : public std::enable_shared_from_this< repo_wrapper >
{
public:
    using branches = std::map< std::string, std::unique_ptr< branch_wrapper > >;
    using remotes = std::map< std::string, std::unique_ptr< git_item_remote > >;

private:
    using remotes_set = std::set< std::string >;

public:
    explicit repo_wrapper( const std::string& repoPath = {},
                   std::unique_ptr< git_item_repo >&& repo_wrapper = nullptr );
	
    // repo operations
    void open_local( const std::string& path );
    void fetch( const git_fetch_options& fetch_opts = GIT_FETCH_OPTIONS_INIT );
    void clone( const std::string& url, const std::string& path, const git_clone_options& cloneOpts = GIT_CLONE_OPTIONS_INIT );
    void close() noexcept;

    // getters
    bool is_valid() const noexcept;
    std::string path() const noexcept;
    std::unique_ptr< branch_wrapper > get_branch( const std::string& ref_name );
    bool get_branches( branches& branchStor, const bool get_remotes = false );

private:
    void read_remotes_list( remotes_set& remotesList );
    void read_branch_commits( branch_wrapper* branch_wrapper);
    void update_remotes(const git_fetch_options& fetch_opts);

private:
    remotes m_remotes;
    std::string m_local_path;
    std::unique_ptr< git_item_repo > m_git_repo;
};

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Aux                   //////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace aux
{
    std::unique_ptr< git_item_str_arr > create_str_arr();
    std::string get_commit_message_str( const commit_wrapper* commit_wrapper );
    std::unique_ptr< git_item_ref > get_reference( const std::string& ref_name, const git_item_repo* repo_wrapper );
    std::unique_ptr< git_item_commit > read_commit(const git_item_repo* repo_wrapper, const git_oid* head );
    std::unique_ptr< git_item_str_arr > get_repo_ref_list( const git_item_repo* repo_wrapper );
    std::string get_branch_name( const std::string& full_branch_name );

    void print_branches( const repo_wrapper::branches& storage );
    void print_commits( const commit_wrapper* commit_wrapper );
    void print_branch_commits( const branch_wrapper* branch_wrapper );
}

}//base

}//git_handler

#endif // GITBASECLASSES_H
