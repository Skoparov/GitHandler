#ifndef GITHANDLER_H
#define GITHANDLER_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <memory>

#include "include/git2.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitItem				    //////////////////////
//////////////////////////////////////////////////////////////////////////////

template<class GitItemType, class git_item_free>
class GitItem
{
private:
	GitItemType* mItem;
	git_item_free mFreeFunc;

public:
	GitItem(git_item_free freeFunc) : mItem(nullptr), mFreeFunc(freeFunc) {}	
	GitItem(GitItemType* item, git_item_free freeFunc) : mItem(item), mFreeFunc(freeFunc){}
	GitItem(GitItem& other) : mItem(other.mItem), mFreeFunc(other.mFreeFunc){ other.mItem = nullptr; }
	
	void setItem(GitItemType* item){
		freeItem();
		mItem = item;
	}

	GitItemType* getItem() const{
		return mItem;
	}	

	GitItemType** getItemPointer(){		
		return &mItem;
	}

	void freeItem(){
		if (mItem){
			mFreeFunc(mItem);
		}
	}

	~GitItem(){
		freeItem();
	}	
};

typedef GitItem<git_repository, void(*)(git_repository*)> GitRepo;
typedef GitItem<git_remote, void(*)(git_remote*)> GitRemote;
typedef GitItem<git_commit, void(*)(git_commit*)> GitCommit;
typedef GitItem<git_reference, void(*)(git_reference*)> GitRef;

typedef std::shared_ptr<GitRepo> GitRepoPtr;
typedef std::shared_ptr<GitRemote> GitRemotePtr;
typedef std::shared_ptr<GitCommit> GitCommitPtr;
typedef std::shared_ptr<GitRef> GitRefPtr;

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitHandler
{	
private:
	int remoteLs(GitRemotePtr remote, std::vector<std::string>& refStor);
	int defaultBranch(GitRemotePtr remote, std::string& branchName);
	std::string remoteName(GitRemotePtr remote);
	std::string refName(GitRefPtr ref);
	git_oid refTarget(GitRefPtr ref);

	//callbacks with params determined by the lib
	static int progress_cb(const char *str, int len, void *data);
	static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);

public:
	GitHandler();
	~GitHandler();

	GitRepoPtr openRepo(const std::string& path);
	int cloneRepo(GitRepoPtr repo, const std::string& url, const std::string& path, const git_clone_options& cloneOpts);
	GitRemotePtr initRemote(GitRepoPtr repo, const std::string& name, const git_fetch_options* opts);
	int fetch(GitRemotePtr remote, const git_fetch_options& opts);

	std::vector<GitCommitPtr> getBranchCommits(GitRefPtr branchRef, GitRepoPtr repo);
};

#endif // GITHANDLER_H
