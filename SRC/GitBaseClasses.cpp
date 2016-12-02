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

commit_wrapper::commit_wrapper( std::unique_ptr< git_item_commit >&& commit ) : m_commit( std::move( commit ) )
{

}

git_oid commit_wrapper::id() const noexcept
{
    git_oid id;

    if( isValid() )
    {
        const git_oid* cId = git_commit_id( m_commit->get() );
        id = *cId;
    }

    return id;
}

git_time commit_wrapper::time() const noexcept
{
    git_time time;

    if( isValid() )
    {
        time = git_commit_author( m_commit->get() )->when;
    }

    return time;
}

std::string commit_wrapper::author() const noexcept
{
    std::string author;

    if( isValid() )
    {
        author = git_commit_author( m_commit->get() )->name;
	}

	return author;
}

std::string commit_wrapper::message() const noexcept
{
    std::string message;

    if( isValid() )
    {
        message = git_commit_message( m_commit->get() );
	}

	return message;
}

bool commit_wrapper::isValid() const noexcept
{
    return m_commit != nullptr && m_commit->get() != nullptr;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

branch_wrapper::branch_wrapper( std::unique_ptr< git_item_ref >&& branch_ref,
                const bool is_remote,
                const std::shared_ptr< repo_wrapper > parent_repo ) :
                m_is_remote( is_remote ),
                m_parent_repo( parent_repo ),
                m_branch_ref( std::move( branch_ref ) )
{

}

void branch_wrapper::add_commit( std::unique_ptr< commit_wrapper >&& commit )
{
    commit_id id( commit->time().time, commit->message() );

    if( !m_commits.count( id ) )
    {
        m_commits.emplace( id, std::move( commit ) );
    }
}

void branch_wrapper::clear_commits() noexcept
{
    m_commits.clear();
}

std::string branch_wrapper::name() const noexcept
{
    std::string name;

    if( is_valid() )
    {
        const char* c_name{ nullptr };
        if ( git_branch_name( &c_name, m_branch_ref->get() ) == 0 )
        {
            name = c_name;
        }
    }

    return name;
}

auto branch_wrapper::commits() const noexcept -> const commit_storage&
{
    return m_commits;
}

bool branch_wrapper::is_remote() const noexcept
{
    return m_is_remote;
}

bool branch_wrapper::is_valid() const noexcept
{
    return m_branch_ref && m_branch_ref.get();
}

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Repo                  //////////////////////
////////////////////////////////////////////////////////////////////////////////

repo_wrapper::repo_wrapper( const std::string& repo_path,
                            std::unique_ptr< git_item_repo >&& repo ) :
                            m_local_path( repo_path),
                            m_git_repo( std::move( repo ) )
{

}

void repo_wrapper::open_local( const std::string& path )
{
    close();
		
    git_repository* r{ nullptr };
    if( git_repository_open( &r, path.c_str() ) != 0 )
    {
        throw std::runtime_error{ "Could not open local repository " + path };
    }

    m_git_repo = factory::git_item_creator::get().create< git_item_repo >( item::type::GIT_REPO, r );
    m_local_path = path;
}

void repo_wrapper::update_remotes( const git_fetch_options& fetch_opts )
{
    if( !is_valid() )
    {
        throw std::logic_error{ "Repository is not valid" };
    }
	
    remotes_set remotes_list;
    read_remotes_list( remotes_list );

    if( !remotes_list.size() )
    {
        return;
    }

    for ( auto remote = m_remotes.begin(); remote != m_remotes.end(); )
    {
        if( !remotes_list.count( remote->first ) )
        {
            m_remotes.erase(remote);
        }
		
        remote++;
    }

    for( auto name : remotes_list )
    {
        auto remoteStor = m_remotes.find(name);
        if ( remoteStor == m_remotes.end() )
        {
            git_remote* remote{ nullptr };
            if( git_remote_lookup( &remote, m_git_repo->get(), name.c_str() ) != 0 )
            {
                throw std::logic_error{ "Could not find remote " + name };
            }
			
            std::string url{ git_remote_url( remote ) };
            std::string name{ git_remote_name( remote ) };

            printf( "Fetching from %s: %s\n", name.c_str(), url.c_str() );

            auto remotePtr = factory::git_item_creator::get().create< git_item_remote >( item::type::GIT_REMOTE, remote );

            m_remotes.emplace( name, std::move( remotePtr ) );
        }
    }
}

void repo_wrapper::fetch( const git_fetch_options& fetch_opts )
{
    if( !is_valid() )
    {
        throw std::logic_error{ "Repository is not valid" };
    }

    update_remotes( fetch_opts );

    for( auto& remote : m_remotes )
    {
        auto arr = aux::create_str_arr();
        if( git_remote_fetch( remote.second->get(), arr->get(), &fetch_opts, nullptr) != 0 )
        {
            throw std::logic_error{ "Could not fetch remote " + remote.first };
        }
    }
}

void repo_wrapper::clone( const std::string& url, const std::string& path, const git_clone_options& clone_opts )
{
    close();

    git_repository* r{ nullptr };
    if( git_clone( &r, url.c_str(), path.c_str(), &clone_opts ) != 0 )
    {
        throw std::logic_error{ "Could not clone repository " + url };
    }

    m_git_repo = factory::git_item_creator::get().create< git_item_repo >( item::type::GIT_REPO, r );
}

void repo_wrapper::close() noexcept
{
    m_git_repo.reset();
    m_local_path.clear();
    m_remotes.clear();
}

void repo_wrapper::read_remotes_list( remotes_set& remotes_list )
{
    if( !is_valid() )
    {
        throw std::logic_error{ "Repository is not valid" };
    }

    auto remote_list = aux::create_str_arr();
    if( git_remote_list( remote_list->get(), m_git_repo->get() ) != 0 )
    {
        throw std::logic_error{ "Could not get remotes list" };
    }

    for( size_t str_num = 0; str_num < remote_list->get()->count; ++str_num )
    {
        remotes_list.insert( remote_list->get()->strings[ str_num ] );
    }				
}

bool repo_wrapper::is_valid() const noexcept
{
    return m_git_repo != nullptr &&
           m_git_repo->get() != nullptr &&
           m_local_path.length();
}

std::string repo_wrapper::path() const noexcept
{
    return m_local_path;
}


void repo_wrapper::read_branch_commits( branch_wrapper* branch )
{
    if( !is_valid() )
    {
        throw std::logic_error{ "Repository is not valid" };
    }
	
    auto oid = const_cast< git_oid* >( git_reference_target( branch->m_branch_ref->get() ) );
    if( !oid )
    {
        throw std::runtime_error{ "Could not get branch ref" };
    }
	   
    git_revwalk* git_walker{ nullptr };

    if( git_revwalk_new( &git_walker, m_git_repo->get() ) != 0 )
    {
        throw std::runtime_error{ "Could not create revwalk" };
    }
	
    auto walker = factory::git_item_creator::get().create< git_item_rev_walk >( item::type::GIT_REV_WALK, git_walker );

    git_revwalk_sorting( walker->get(), GIT_SORT_TOPOLOGICAL );
    git_revwalk_push( walker->get(), oid );
			   
    while ( git_revwalk_next( oid, walker->get() ) == 0 )
    {
        git_commit* commit{ nullptr };
        if( git_commit_lookup( &commit, m_git_repo->get(), oid ) != 0 )
        {
            branch->clear_commits();
            throw std::logic_error{ "Could not read branch commits" };
        }

        auto commit_ptr = factory::git_item_creator::get().create< git_item_commit >( item::type::GIT_COMMIT, commit );
        auto ptr = std::make_unique< commit_wrapper >( std::move( commit_ptr ) );

        branch->add_commit( std::move( ptr ) );
    }
}

bool repo_wrapper::get_branches( branches& branchStorage, const bool getRemotes )
{
    if( !is_valid() )
    {
        throw std::logic_error{ "Repository is not valid" };
    }

    auto arr = aux::get_repo_ref_list( m_git_repo.get() );
    if( !arr )
    {
        throw std::logic_error{ "Failed to get repo's refs list" };
    }

    for( size_t ref_num = 0; ref_num < arr->get()->count; ++ref_num )
    {
        std::string ref_name = arr->get()->strings[ ref_num ];

        auto ref_ptr = aux::get_reference( ref_name, m_git_repo.get() );
        if ( !ref_ptr)
        {
            branchStorage.clear();
            throw std::logic_error{ "Could not get reference for " + ref_name };
        }

        using check_func = int (*)( const git_reference* );

        check_func checkFunc = getRemotes?
                              git_reference_is_remote : git_reference_is_branch;

        if( checkFunc( ref_ptr->get() ) )
        {
            auto branch = std::make_unique< branch_wrapper >( std::move( ref_ptr ), true );
            branchStorage.emplace( branch->name(), std::move( branch ) );
        }
    }

    for( const auto& branch : branchStorage )
    {
        //printf( "Reading branch commits for branch: %s\r", branch.first.c_str() );
        read_branch_commits( branch.second.get() );
    }

    printf("\n");

    return true;
}

std::unique_ptr< branch_wrapper > repo_wrapper::get_branch( const std::string& ref_name )
{
    auto ref_ptr = aux::get_reference( ref_name, m_git_repo.get() );
    if ( ref_ptr )
    {
        int is_remote{ false };
        int is_local{ git_reference_is_branch( ref_ptr->get() ) };

        if( !is_local )
        {
            is_remote = git_reference_is_remote( ref_ptr->get() );
        }

        if( is_local || is_remote )
        {
            auto branch = std::make_unique< branch_wrapper >( std::move( ref_ptr ), is_remote );

            read_branch_commits( branch.get() );
            return branch;
        }
    }
	
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////                   Aux                   //////////////////////
////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< git_item_str_arr > aux::create_str_arr()
{
    auto remoteList = factory::git_item_creator::get().create< git_item_str_arr >( item::type::GIT_STR_ARR );
    remoteList->get()->count = 0;
    return remoteList;
}

std::string aux::get_commit_message_str( const commit_wrapper* commit )
{
    if( !commit )
    {
        return std::string{};
    }

    std::string raw_message{ commit->message() };
    std::vector< std::string > parts;

    boost::split( parts, raw_message, boost::is_any_of( "\n" ) );
    raw_message = boost::join_if( parts,
                                 "\n",
                                 []( const std::string& str ) -> bool
                                 { return !str.empty(); } );


    auto commit_time = commit->time();

    using namespace boost::posix_time;

    time_duration td{ 0, 0, 0, time_duration::ticks_per_second() * ( commit_time.time + commit_time.offset * 60 ) };
    ptime dtime{ boost::gregorian::date{ 1970, 1, 1 }, td };

    std::string message{ ( boost::format( "[%s]\n%s\n%s\n\n" )
                           % dtime
                           % raw_message
                           % commit->author() ).str() };

    return message;
}

std::unique_ptr< git_item_ref > aux::get_reference( const std::string& ref_name, const git_item_repo* repo )
{
    git_reference* ref{ nullptr };
    if( git_reference_lookup( &ref, repo->get(), ref_name.c_str() ) == 0 )
    {
        auto ref_ptr = factory::git_item_creator::get().create< git_item_ref >( item::type::GIT_REF, ref );
        return ref_ptr;
    }

    return nullptr;
}

std::unique_ptr< git_item_commit > aux::read_commit( const git_item_repo* repo, const git_oid* head )
{
    git_commit* commit{ nullptr };

    if ( repo && head &&
         git_commit_lookup( &commit, repo->get(), head ) == 0 )
    {
        auto commit_ptr = factory::git_item_creator::get().create< git_item_commit >( item::type::GIT_COMMIT, commit );
        return commit_ptr;
    }

    return nullptr;
}

std::unique_ptr< git_item_str_arr > aux::get_repo_ref_list( const git_item_repo* repo )
{
    std::unique_ptr< git_item_str_arr > result;

    if( repo )
    {
        result = aux::create_str_arr();

        if( !result || git_reference_list( result->get(), repo->get() ) != 0 )
        {
            result.reset();
        }
    }

    return result;
}

std::string aux::get_branch_name( const std::string& fullBranchName )
{
    //TODO
    return fullBranchName;
}

void aux::print_branches(const repo_wrapper::branches& storage)
{
    printf( "\n-------------------\n" );
    printf( "%s\n", "BRANCHES" );
    printf( "-------------------\n\n" );
	
    for( const auto& branch : storage )
    {
        printf( "%s\n", branch.first.c_str() );
    }
}

void aux::print_commits(const commit_wrapper* commit)
{
    std::string message = aux::get_commit_message_str( commit );
    printf( "%s", message.c_str() );
}

void aux::print_branch_commits( const branch_wrapper* branch )
{
    if( branch )
    {
        printf( "\n-------------------\n" );
        printf( "%s\n", branch->name().c_str() );
        printf( "-------------------\n\n" );

        auto& commits = branch->commits();
        for( const auto& commit : commits )
        {
            print_commits( commit.second.get() );
        }
    }
}

}// git_base_classes

}// git_handler
