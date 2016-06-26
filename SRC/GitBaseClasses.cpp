#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "GitBaseClasses.h"
#include "GitItem.cpp"

namespace git_handler
{

namespace base
{

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Commit::Commit( std::unique_ptr< GitCommit >&& commit ) : mCommit( std::move( commit ) )
{

}

git_oid Commit::id() const noexcept
{
    git_oid id;

    if ( isValid() )
    {
        const git_oid* cId = git_commit_id( mCommit->get() );
        id = *cId;
    }

    return id;
}

git_time Commit::time() const noexcept
{
    git_time time;

    if ( isValid() )
    {
        time = git_commit_author( mCommit->get() )->when;
    }

    return time;
}

std::string Commit::author() const noexcept
{
    std::string author;

    if ( isValid() )
    {
        author = git_commit_author( mCommit->get() )->name;
	}

	return author;
}

std::string Commit::message() const noexcept
{
    std::string message;

    if ( isValid() )
    {
        message = git_commit_message( mCommit->get() );
	}

	return message;
}

bool Commit::isValid() const noexcept
{
    return mCommit != nullptr && mCommit->get() != nullptr;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

Branch::Branch( std::unique_ptr< GitRef >&& branchRef,
                const bool isRemote,
                const std::shared_ptr< Repo > parentRepo ) :
                mBranchRef( std::move( branchRef ) ),
                mIsRemote( isRemote ),
                mParentRepo( parentRepo )
{

}

void Branch::addCommit( std::unique_ptr< Commit >&& commit )
{
    CommitId id( commit->time().time, commit->message() );

    if (!mCommits.count(id))
    {
        mCommits.emplace( id, std::move( commit ) );
    }
}

void Branch::clearCommits() noexcept
{
    mCommits.clear();
}

std::string Branch::name() const noexcept
{
    std::string name;

    if (isValid())
    {
        const char* c_name;
        if ( git_branch_name( &c_name, mBranchRef->get() ) == 0 )
        {
            name = c_name;
        }
    }

    return name;
}

auto Branch::commits() const noexcept -> const CommitStorage&
{
    return mCommits;
}

bool Branch::isRemote() const noexcept
{
    return mIsRemote;
}

bool Branch::isValid() const noexcept
{
    return mBranchRef != nullptr &&
           mBranchRef.get() != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Repo                  //////////////////////
////////////////////////////////////////////////////////////////////////////////

Repo::Repo( const std::string& repoPath,
            std::unique_ptr< GitRepo >&& repo ) :
            mLocalPath( repoPath),
            mGitRepo( std::move( repo ) )
{

}

bool Repo::openLocal( const std::string& path )
{
    closeRepo();
		
    git_repository* r;
    if ( git_repository_open( &r, path.c_str() ) != 0 )
    {
        return false;
    }

    mGitRepo = factory::GitItemCreator::get().create< GitRepo >( item::Type::GIT_REPO, r );
    mLocalPath = path;

    return true;
}

bool Repo::updateRemotes( const git_fetch_options& fetch_opts )
{
    if ( !isValid() )
    {
        return false;
    }
	
    RemotesList remotesList;
    if ( !readRemotesList(remotesList) )
    {
        return false;
    }

    if ( !remotesList.size() )
    {
        return true;
    }

    for ( auto remote = mRemotes.begin(); remote != mRemotes.end(); )
    {
        if ( !remotesList.count( remote->first ) )
        {
            mRemotes.erase(remote);
        }
		
        remote++;
    }
		
    bool result = true;
    for ( auto name : remotesList )
    {
        auto remoteStor = mRemotes.find(name);
        if ( remoteStor == mRemotes.end() )
        {
            git_remote* remote = nullptr;
            if ( git_remote_lookup( &remote, mGitRepo->get(), name.c_str() ) != 0)
            {
                return false;
            }
			
            std::string url = git_remote_url( remote );
            std::string name = git_remote_name( remote );

            printf("Fetching from %s: %s\n", name.c_str(), url.c_str());

            auto remotePtr = factory::GitItemCreator::get().create< GitRemote >( item::Type::GIT_REMOTE, remote );

            mRemotes.emplace( name, std::move( remotePtr ) );
        }
    }

    return true;
}

bool Repo::fetch( const git_fetch_options& fetch_opts )
{
    if ( !isValid() )
    {
        return false;
    }

    if ( !updateRemotes(fetch_opts) )
    {
        return false;
    }

    for ( auto& remote : mRemotes )
    {
        auto arr = Aux::createStrArr();
        if (arr == nullptr)
        {
            return false;
        }

        if ( git_remote_fetch( remote.second->get(), arr->get(), &fetch_opts, nullptr) != 0 )
        {
            return false;
        }
    }

    return true;
}

bool Repo::clone( const std::string& url, const std::string& path, const git_clone_options& cloneOpts )
{
    closeRepo();

    git_repository* r;
    if ( git_clone( &r, url.c_str(), path.c_str(), &cloneOpts ) != 0 )
    {
        return false;
    }

    mGitRepo = factory::GitItemCreator::get().create< GitRepo >( item::Type::GIT_REPO, r );
	    
    return true;
}

void Repo::closeRepo() noexcept
{
    mGitRepo.reset();
    mLocalPath = {};
    mRemotes.clear();
}

bool Repo::readRemotesList(RemotesList& remotesList)
{
    if (!isValid()){
        return false;
    }

    auto remoteList = Aux::createStrArr();
    if ( remoteList == nullptr )
    {
        return false;
    }

    if ( git_remote_list( remoteList->get(), mGitRepo->get() ) != 0 )
    {
        return false;
    }

    for ( size_t strNum = 0; strNum < remoteList->get()->count; ++strNum )
    {
        remotesList.insert( remoteList->get()->strings[strNum] );
    }
				
    return true;
}

bool Repo::isValid() const noexcept
{
    return mGitRepo != nullptr &&
           mGitRepo->get() != nullptr &&
           mLocalPath.length();
}

std::string Repo::path() const noexcept
{
    return mLocalPath;
}


bool Repo::readBranchCommits( Branch* branch )
{
    if ( !isValid() )
    {
        return false;
    }
	
    git_oid* oid = const_cast< git_oid* >( git_reference_target( branch->mBranchRef->get() ) );
    if (oid == nullptr)
    {
        return false;
    }
	   
    git_revwalk *gitWalker;

    if ( git_revwalk_new( &gitWalker, mGitRepo->get() ) != 0 )
    {
        return false;
    }
	
    auto walker = factory::GitItemCreator::get().create< GitRevWalk >( item::Type::GIT_REV_WALK, gitWalker );

    git_revwalk_sorting( walker->get(), GIT_SORT_TOPOLOGICAL );
    git_revwalk_push( walker->get(), oid );
			
    bool result = true;
    std::vector< git_oid > iDs;

    while ( git_revwalk_next( oid, walker->get() ) == 0)
    {
        git_commit* commit;
        if ( git_commit_lookup( &commit, mGitRepo->get(), oid ) != 0 )
        {
            result = false;
            break;
        }

        auto commitPtr = factory::GitItemCreator::get().create< GitCommit >( item::Type::GIT_COMMIT, commit );
        auto cPtr = std::make_unique< Commit >( std::move( commitPtr ) );

        branch->addCommit( std::move( cPtr ) );
    }

    if (!result)
    {
        branch->clearCommits();
    }

    return result;
}

bool Repo::getBranches( BranchStorage& branchStorage, const bool getRemotes )
{
    if ( !isValid() )
    {
        printf("Get local branches: Repo not initialized\n");
        return false;
    }

    auto arr = Aux::getRepoRefList( mGitRepo.get() );
    if ( arr == nullptr )
    {
        printf( "Get local branches: Failed to get repo's refs list\n" );
        return false;
    }

    for ( size_t refNum = 0; refNum < arr->get()->count; ++refNum )
    {
        std::string refName = arr->get()->strings[refNum];

        auto refPtr = Aux::getReference( refName, mGitRepo.get() );
        if ( refPtr == nullptr )
        {
            branchStorage.clear();
            return false;
        }

        using CheckFunc = int (*)(const git_reference *);

        CheckFunc checkFunc = getRemotes?
                              git_reference_is_remote : git_reference_is_branch;

        if ( checkFunc( refPtr->get() ) )
        {
            auto branch = std::make_unique<Branch>( std::move( refPtr ), true);
            branchStorage.emplace( branch->name(), std::move( branch ) );
        }
    }

    for ( const auto& branch : branchStorage )
    {
        printf("Reading branch commits for branch: %s\r", branch.first.c_str());
        readBranchCommits( branch.second.get() );
    }

    printf("\n");

    return true;
}

std::unique_ptr< Branch > Repo::getBranch( const std::string& refName )
{
    auto refPtr = Aux::getReference( refName, mGitRepo.get() );
    if ( refPtr != nullptr )
    {
        int isRemote = false;
        int isLocal = git_reference_is_branch( refPtr->get() );

        if (!isLocal)
        {
            isRemote = git_reference_is_remote(refPtr->get());
        }

        if (isLocal || isRemote)
        {
            auto branch = std::make_unique< Branch >( std::move( refPtr ), isRemote );

            if ( !readBranchCommits( branch.get() ) )
            {
                branch.reset();
            }

            return branch;
        }
    }
	
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Aux                   //////////////////////
////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< GitStrArr > Aux::createStrArr()
{
    auto remoteList = factory::GitItemCreator::get().create< GitStrArr >( item::Type::GIT_STR_ARR );
    remoteList->get()->count = 0;
    return remoteList;
}

std::string Aux::getCommitMessageStr( const Commit* commit )
{
    if (commit == nullptr)
    {
        return std::string{};
    }

    std::string rawMessage = commit->message();
    std::vector< std::string > parts;

    boost::split( parts, rawMessage, boost::is_any_of("\n") );
    rawMessage = boost::join_if( parts,
                                 "\n",
                                 [](const std::string& str)->bool
                                 { return str.length() != 0; } );


    auto commitTime = commit->time();

    using namespace boost::posix_time;

    time_duration td( 0, 0, 0, time_duration::ticks_per_second() * ( commitTime.time + commitTime.offset * 60 ) );
    ptime dtime = ptime(boost::gregorian::date(1970, 1, 1), td);

    std::string message = ( boost::format("[%s]\n%s\n%s\n\n")
                            % dtime
                            % rawMessage
                            % commit->author() ).str();

    return message;
}

std::unique_ptr< GitRef > Aux::getReference( const std::string& refName, const GitRepo* repo )
{
    git_reference* ref = nullptr;
    if (git_reference_lookup(&ref, repo->get(), refName.c_str()) == 0)
    {
        auto refPtr = factory::GitItemCreator::get().create< GitRef >( item::Type::GIT_REF, ref );
        return refPtr;
    }

    return nullptr;
}

std::unique_ptr< GitCommit > Aux::readCommit( const GitRepo* repo, const git_oid *head )
{
    git_commit* gitCommit;

    if ( repo != nullptr &&
         head != nullptr &&
         git_commit_lookup( &gitCommit, repo->get(), head ) == 0)
    {
        auto commit = factory::GitItemCreator::get().create< GitCommit >( item::Type::GIT_COMMIT, gitCommit );
        return commit;
    }

    return nullptr;
}

std::unique_ptr< GitStrArr > Aux::getRepoRefList( const GitRepo* repo )
{
    std::unique_ptr< GitStrArr > result;

    if ( repo != nullptr )
    {
        result = Aux::createStrArr();

        if ( result == nullptr ||
             git_reference_list( result->get(), repo->get() ) != 0 )
        {
            result.reset();
        }
    }

    return result;
}

std::string Aux::getBranchName( const std::string& fullBranchName )
{
    //TODO
    return fullBranchName;
}

void Aux::printBranches(const Repo::BranchStorage& storage)
{
    printf( "\n-------------------\n" );
    printf( "%s\n", "BRANCHES" );
    printf( "-------------------\n\n" );
	
    for ( auto& branch : storage )
    {
        printf( "%s\n", branch.first.c_str() );
    }
}

void Aux::printCommit(const Commit* commit)
{
    std::string message = Aux::getCommitMessageStr( commit );
    printf( "%s", message.c_str() );
}

void Aux::printBranchCommits( const Branch* branch )
{
    if ( branch != nullptr )
    {
        printf( "\n-------------------\n" );
        printf( "%s\n", branch->name().c_str() );
        printf( "-------------------\n\n" );

        auto& commits = branch->commits();
        for ( const auto& commit : commits )
        {
            printCommit( commit.second.get() );
        }
    }
}

}// git_base_classes

}// git_handler
