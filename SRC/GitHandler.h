#ifndef GITHANDLER_H
#define GITHANDLER_H

#include "GitBaseClasses.h"

typedef std::map<string, RepoPtr> RepoStorage;

//////////////////////////////////////////////////////////////////////////////
///////////////                GitHandler               //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitHandler
{	
	typedef std::weak_ptr<Repo> CurrentRepoPtr;
	typedef std::map<string, vector<BranchPtr>> NewBranchStorage;
	typedef std::map<std::pair<string, string>, vector<CommitPtr>> NewCommitStorage;

public:
	GitHandler();
	~GitHandler();

	bool addRepo(const RepoPtr repo);
	bool update();
	void clear();
		
	RepoPtr getRepo(const string& path) const;
	const RepoStorage& getRepos() const;

	NewBranchStorage newBranches();
	NewCommitStorage newCommits();	

private:	
	bool registerGitItemTypes();

	//callbacks with params determined by the lib
	static int progress_cb(const char *str, int len, void *data);
	static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);	
	
private:
	RepoStorage mRepos;
	static CurrentRepoPtr mCurrentRepo;

	static NewBranchStorage mNewBranches;
	static NewCommitStorage mNewCommits;
};

#endif // GITHANDLER_H
