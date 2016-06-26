#include "GitDeleters.h"

namespace git_handler
{

namespace deleters
{

template<>
void deleteItem( git_repository* repo )
{
    if (repo != nullptr)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

template<>
void deleteItem( git_remote* remote )
{
    if (remote != nullptr)
    {
        git_remote_disconnect(remote);
        git_remote_free(remote);
        remote = nullptr;
    }
}

template<>
void deleteItem( git_commit* commit )
{
    if (commit != nullptr)
    {
        git_commit_free(commit);
        commit = nullptr;
    }
}

template<>
void deleteItem( git_reference* refNum )
{
    if (refNum != nullptr)
    {
        git_reference_free(refNum);
        refNum = nullptr;
    }
}

template<>
void deleteItem( git_strarray* arr )
{
    if (arr != nullptr)
    {
        if (arr->count){
            git_strarray_free(arr);
        }

        delete arr;
        arr = nullptr;
    }
}

template<>
void deleteItem( git_revwalk* rv )
{
    if (rv != nullptr)
    {
        git_revwalk_free(rv);
        rv = nullptr;
    }
}

}//deleters

}//git_handler
