#ifndef DELETERS_H
#define DELETERS_H

#include "include/git2.h"

static void deleteRepo(git_repository* repo)
{
	git_repository_free(repo);
}

static void deleteRemote(git_remote* remote)
{
	git_remote_free(remote);
}

static void deleteCommit(git_commit* commit)
{
	git_commit_free(commit);
}


static void deleteRef(git_reference* ref)
{
	git_reference_free(ref);
}

static void deleteStrArr(git_strarray* arr)
{
	git_strarray_free(arr);
	delete arr;
}

#endif