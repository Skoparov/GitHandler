#ifndef DELETERS_H
#define DELETERS_H

#include "include/git2.h"

static void deleteRepo(git_repository* repo)
{
	if (repo != nullptr){
		git_repository_free(repo);
	}
}

static void deleteRemote(git_remote* remote)
{
	if (remote != nullptr)
	{
		git_remote_disconnect(remote);
		git_remote_free(remote);
	}
}

static void deleteCommit(git_commit* commit)
{
	if (commit != nullptr){
		git_commit_free(commit);
	}
}


static void deleteRef(git_reference* refNUm)
{
	if (refNUm != nullptr){
		git_reference_free(refNUm);
	}	
}

static void deleteStrArr(git_strarray* arr)
{
	if (arr != nullptr)
	{
		if (arr->count){
			git_strarray_free(arr);
		}

		delete arr;
	}	
}

#endif