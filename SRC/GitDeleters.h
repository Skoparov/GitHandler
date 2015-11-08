#ifndef GITDELETERS_H
#define GITDELETERS_H

#include "include/git2.h"

static void deleteRepo(git_repository* repo)
{
	if (repo != nullptr)
	{
		git_repository_free(repo);
		repo = nullptr;
	}
}

static void deleteRemote(git_remote* remote)
{
	if (remote != nullptr)
	{
		git_remote_disconnect(remote);
		git_remote_free(remote);
		remote = nullptr;
	}
}

static void deleteCommit(git_commit* commit)
{
	if (commit != nullptr)
	{
		git_commit_free(commit);
		commit = nullptr;
	}
}

static void deleteRef(git_reference* refNum)
{
	if (refNum != nullptr)
	{
		git_reference_free(refNum);
		refNum = nullptr;
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
		arr = nullptr;
	}	
}

static void deleteRevWalk(git_revwalk* rv)
{
	if (rv != nullptr)
	{
		git_revwalk_free(rv);
		rv = nullptr;
	}
}

#endif //GITDELETERS_H