#include "GitDeleters.h"

namespace git_handler
{

namespace deleters
{

template<>
void delete_item( git_repository* repo )
{
    if( repo != nullptr )
    {
        git_repository_free( repo );
        repo = nullptr;
    }
}

template<>
void delete_item( git_remote* remote )
{
    if( remote != nullptr )
    {
        git_remote_disconnect( remote );
        git_remote_free( remote );
        remote = nullptr;
    }
}

template<>
void delete_item( git_commit* commit )
{
    if( commit != nullptr )
    {
        git_commit_free( commit );
        commit = nullptr;
    }
}

template<>
void delete_item( git_reference* ref_num )
{
    if( ref_num != nullptr )
    {
        git_reference_free( ref_num );
        ref_num = nullptr;
    }
}

template<>
void delete_item( git_strarray* arr )
{
    if( arr != nullptr )
    {
        if( arr->count )
        {
            git_strarray_free( arr );
        }

        delete arr;
        arr = nullptr;
    }
}

template<>
void delete_item( git_revwalk* rv )
{
    if( rv != nullptr )
    {
        git_revwalk_free( rv );
        rv = nullptr;
    }
}

}//deleters

}//git_handler
