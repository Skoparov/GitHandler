#include "GitHandler.h"
#include "GitItem.cpp"

namespace git_handler
{

////////////////////////////////////////////////////////////////////////////////
/////////////////                GitHandler               //////////////////////
////////////////////////////////////////////////////////////////////////////////

base::Repo* GitHandler::mCurrentRepo;
GitHandler::NewBranchStorage GitHandler::mNewBranches;
GitHandler::NewCommitStorage GitHandler::mNewCommits;
GitHandler::Credentials GitHandler::mCredentials;

GitHandler::GitHandler()
{
    git_libgit2_init();
    registerGitItemTypes();
}

bool GitHandler::addRepo( std::unique_ptr< base::Repo >&& repo, const std::string& username, const std::string& pass)
{
    if ( repo->isValid() )
    {
        mRepos.emplace( repo->path(), std::move( repo )  );
        mCredentials.emplace( repo->path(), std::make_pair( username, pass ) );
        return true;
    }

    return false;
}

bool GitHandler::update() noexcept
{
    bool result = true;

    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.update_tips = &update_cb;
    fetch_opts.callbacks.sideband_progress = &progress_cb;
    fetch_opts.callbacks.credentials = &cred_acquire_cb;

    for ( auto& repo : mRepos )
    {
        result &= repo.second->fetch( fetch_opts );
    }

    return result;
}

void GitHandler::clear() noexcept
{
    mRepos.clear();
    mNewBranches.clear();
    mNewCommits.clear();
}

base::Repo* GitHandler::getRepo( const std::string& path ) const noexcept
{
    auto repoIter = mRepos.find( path );
    if( repoIter != mRepos.end() )
    {
        return repoIter->second.get();
    }

    return nullptr;
}

auto GitHandler::getRepos() const noexcept -> const RepoStorage&
{
    return mRepos;
}

//auto GitHandler::newBranches() -> NewBranchStorage
//{
//    return NewBranchStorage( std::move(mNewBranches) );
//}

//auto GitHandler::newCommits() -> NewCommitStorage
//{
//    return NewCommitStorage( std::move( mNewCommits ) );
//}

int GitHandler::progress_cb( const char *str, int len, void *data )
{
    //printf("remote: %.*s", len, str);
    //fflush(stdout); /* We don't have the \n to force the flush */
    return 0;
}

int GitHandler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
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

int GitHandler::cred_acquire_cb( git_cred **out, const char* url, const char* username_from_url, unsigned int allowed_typed, void* data )
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
std::unique_ptr< factory::GitItemFactoryIf > createFactory()
{
    return std::make_unique< factory::GitItemFactory< GitItemType > >();
}

bool GitHandler::registerGitItemTypes()
{
    bool ok = true;
	
    auto& c = factory::GitItemCreator::get();

    ok &= c.registerItemType < git_repository > ( item::Type::GIT_REPO,     std::move( createFactory< git_repository >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_REMOTE,   std::move( createFactory< git_remote >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_COMMIT,   std::move( createFactory< git_commit >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_REF,      std::move( createFactory< git_reference >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_STR_ARR,  std::move( createFactory< git_strarray >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_REV_WALK, std::move( createFactory< git_repository >() ) );
//    ok &= c.registerItemType < git_repository > ( item::Type::GIT_REPO,     std::move( createFactory< git_revwalk >() ) );

    return ok;
}

GitHandler::~GitHandler()
{
    clear();
    git_libgit2_shutdown();
}

} //git_handler
