#ifndef GITITEM_H
#define GITITEM_H

#include "memory"
#include "GitDeleters.h"
#include <stdarg.h>

enum GitItemType
{
	GIT_REPO,
	GIT_REMOTE,
	GIT_COMMIT,
	GIT_REF,
	GIT_STR_ARR,
	GIT_REV_WALK
};

class Item;
typedef std::shared_ptr<Item> ItemPtr;

//////////////////////////////////////////////////////////////////////////////
///////////////				     GitItem				//////////////////////
//////////////////////////////////////////////////////////////////////////////

class Item
{
public: 
	virtual ~Item() {}; 
};

template<class GitItemType, class Deleter>
class GitItem : public Item
{
	typedef std::shared_ptr<GitItemType> ItemPtr;

public:
	GitItem(Deleter del) : mDeleter(del) {}

	void initItem(){
		mItem = ItemPtr(new GitItemType, mDeleter);
	}

	ItemPtr item() {
		return mItem;
	};

	GitItemType* gitItem(){
		return mItem.get();
	}	

	void setItem(GitItemType* item){		
		mItem.reset(item, mDeleter);	
	}
	
	bool isValid(){
		return mItem != nullptr; 
	}	

private:
	std::shared_ptr<GitItemType> mItem;
	Deleter mDeleter;
};

typedef GitItem<git_repository, void(*)(git_repository*)> GitRepo;
typedef GitItem<git_remote,     void(*)(git_remote*)>     GitRemote;
typedef GitItem<git_commit,     void(*)(git_commit*)>     GitCommit;
typedef GitItem<git_reference,  void(*)(git_reference*)>  GitRef;
typedef GitItem<git_strarray,   void(*)(git_strarray*)>   GitStrArr;
typedef GitItem<git_revwalk,    void(*)(git_revwalk*)>    GitRevWalk;

typedef std::shared_ptr<GitRepo>    GitRepoPtr;
typedef std::shared_ptr<GitRemote>  GitRemotePtr;
typedef std::shared_ptr<GitCommit>  GitCommitPtr;
typedef std::shared_ptr<GitRef>     GitRefPtr;
typedef std::shared_ptr<GitStrArr>  GitStrArrPtr;
typedef std::shared_ptr<GitRevWalk> GitRevWalkPtr;

#endif