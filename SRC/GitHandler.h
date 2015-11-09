#ifndef GITHANDLER_H
#define GITHANDLER_H

#include "GitBaseClasses.h"

using std::pair;

typedef std::map<string, RepoPtr> RepoStorage;

//////////////////////////////////////////////////////////////////////////////
///////////////                GitHandler               //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitHandler
{	
	typedef std::weak_ptr<Repo> CurrentRepoPtr;
	typedef std::map<string, vector<BranchPtr>> NewBranchStorage;
	typedef std::map<std::pair<string, string>, vector<CommitPtr>> NewCommitStorage;
	typedef std::map<string, pair<string, string>> Credentials;

public:
	GitHandler();
	~GitHandler();

	bool addRepo(const RepoPtr repo, const string username, const string pass);
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
	static int cred_acquire_cb(git_cred **out, const char* url, const char* username_from_url, unsigned int allowed_typed, void* data);

private:
	RepoStorage mRepos;

	static Credentials mCredentials;
	static CurrentRepoPtr mCurrentRepo;	

	static NewBranchStorage mNewBranches;
	static NewCommitStorage mNewCommits;
};

#endif // GITHANDLER_H
