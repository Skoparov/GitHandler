#include "GitHandler.h"
#include "GitItem.cpp"

namespace git_handler
{

////////////////////////////////////////////////////////////////////////////////
/////////////////                GitHandler               //////////////////////
////////////////////////////////////////////////////////////////////////////////

base::repo_wrapper* git_handler::mCurrentRepo;
git_handler::new_braches git_handler::m_new_branches;
git_handler::new_commits git_handler::m_new_commits;
git_handler::credentials git_handler::mCredentials;

git_handler::git_handler()
{
    git_libgit2_init();
    register_git_items();
}

bool git_handler::add_repo( std::unique_ptr< base::repo_wrapper >&& repo, const std::string& username, const std::string& pass)
{
    if ( repo->is_valid() )
    {
        m_repos.emplace( repo->path(), std::move( repo )  );
        mCredentials.emplace( repo->path(), std::make_pair( username, pass ) );
        return true;
    }

    return false;
}

void git_handler::update() noexcept
{   
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.update_tips = &update_cb;
    fetch_opts.callbacks.sideband_progress = &progress_cb;
    fetch_opts.callbacks.credentials = &cred_acquire_cb;

    for ( auto& repo : m_repos )
    {
       repo.second->fetch( fetch_opts );
    }
}

void git_handler::clear() noexcept
{
    m_repos.clear();
    m_new_branches.clear();
    m_new_commits.clear();
}

base::repo_wrapper* git_handler::getRepo( const std::string& path ) const noexcept
{
    auto rep_iter = m_repos.find( path );
    if( rep_iter != m_repos.end() )
    {
        return rep_iter->second.get();
    }

    return nullptr;
}

auto git_handler::get_repos() const noexcept -> const repos&
{
    return m_repos;
}

//auto GitHandler::newBranches() -> NewBranchStorage
//{
//    return NewBranchStorage( std::move(mNewBranches) );
//}

//auto GitHandler::newCommits() -> NewCommitStorage
//{
//    return NewCommitStorage( std::move( mNewCommits ) );
//}

int git_handler::progress_cb( const char *str, int len, void *data )
{
    //printf("remote: %.*s", len, str);
    //fflush(stdout); /* We don't have the \n to force the flush */
    return 0;
}

int git_handler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
//    if ( !mCurrentRepo )
//    {
//        return 1;
//    }

//    if ( git_oid_iszero( oldHead ) ) // new branch
//    {
//        auto ref = base::Aux::getReference( refname, currRepo->get() );
//        if ( !ref )
//        {
//            return 1;
//        }

//        auto branchPtr = std::make_unique< Branch >( ref, true );

//        auto branchStor = mNewBranches.find( currRepo->path() );
//        if ( branchStor == mNewBranches.end() )
//        {
//            mNewBranches.emplace( currRepo->path(), std::vector< base::Branch* >() );
//            branchStor = mNewBranches.find( currRepo->path() );
//        }

//        branchStor->second.push_back( branchPtr );
//    }
//    else //new commit
//    {
//        auto key = std::make_pair(currRepo->path(), refname);
//        auto storage = mNewCommits.find(key);
//        if (storage == mNewCommits.end())
//        {
//            mNewCommits.insert(std::make_pair(key, vector<CommitPtr>()));
//            storage = mNewCommits.find(key);
//        }

//        auto branch = currRepo->getBranch(refname);
//        if (branch == nullptr){
//            return 1;
//        }

//        const CommitStorage& commits = branch->commits();
//        auto oldHeadCommit = std::find_if(commits.begin(), commits.end(),
//                                          [oldHead](CommitStorage::value_type commit)->bool
//                                          {
//                                            const auto& id = commit.second->id();
//                                            return git_oid_equal(oldHead, &id);
//                                          });

//        if (oldHeadCommit != commits.end()){
//            oldHeadCommit++;
//        }
			
//        for (auto commitIt = oldHeadCommit; commitIt != commits.end(); ++commitIt){
//            storage->second.push_back(commitIt->second);
//        }
//    }
	
    return 0;
}

int git_handler::cred_acquire_cb( git_cred **out, const char* url, const char* username_from_url, unsigned int allowed_typed, void* data )
{
    int res = 1;

    if ( !mCurrentRepo )
    {
        return res;
    }

    const auto& credentials = mCredentials.find( mCurrentRepo->path() );
    if (credentials != mCredentials.end())
    {
        res = git_cred_userpass_plaintext_new( out, credentials->second.first.c_str(), credentials->second.second.c_str() );
        //int res = git_cred_ssh_key_new(out, "git", "C:\\Users\\Sergey\\\.ssh\\id_rsa.pub", "C:\\Users\\Sergey\\\.ssh\\id_rsa", "221289");
    }

    printf("checking key: %d\n", res);
    return res;
}

template< class GitItemType >
std::unique_ptr< factory::igit_item_factory > create_factory()
{
    return std::make_unique< factory::git_item_factory< GitItemType > >();
}

bool git_handler::register_git_items()
{
    bool ok = true;
	
    auto& c = factory::git_item_creator::get();

    ok &= c.register_item_type < git_repository > ( item::type::GIT_REPO,     std::move( create_factory< git_repository >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_REMOTE,   std::move( create_factory< git_remote >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_COMMIT,   std::move( create_factory< git_commit >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_REF,      std::move( create_factory< git_reference >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_STR_ARR,  std::move( create_factory< git_strarray >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_REV_WALK, std::move( create_factory< git_repository >() ) );
    ok &= c.register_item_type < git_repository > ( item::type::GIT_REPO,     std::move( create_factory< git_revwalk >() ) );

    return ok;
}

git_handler::~git_handler()
{
    clear();
    git_libgit2_shutdown();
}

} //git_handler
