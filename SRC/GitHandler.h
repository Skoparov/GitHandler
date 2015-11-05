#ifndef GITHANDLER_H
#define GITHANDLER_H

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <functional>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "GitItemFactory.h"

using namespace boost::posix_time;

class Commit;
class Branch;
class Repo;

typedef GitItem<git_repository, void(*)(git_repository*)> GitRepo;
typedef GitItem<git_remote,     void(*)(git_remote*)>     GitRemote;
typedef GitItem<git_commit,     void(*)(git_commit*)>     GitCommit;
typedef GitItem<git_reference,  void(*)(git_reference*)>  GitRef;
typedef GitItem<git_strarray,   void(*)(git_strarray*)>   GitStrArr;

typedef std::shared_ptr<GitRepo>   GitRepoPtr;
typedef std::shared_ptr<GitRemote> GitRemotePtr;
typedef std::shared_ptr<GitCommit> GitCommitPtr;
typedef std::shared_ptr<GitRef>    GitRefPtr;
typedef std::shared_ptr<GitStrArr> GitStrArrPtr;

typedef std::shared_ptr<Commit>    CommitPtr;
typedef std::shared_ptr<Branch>    BranchPtr;
typedef std::shared_ptr<Repo>      RepoPtr;
typedef std::weak_ptr<Branch>      ParentBranchPtr;
typedef std::weak_ptr<Repo>        ParentRepoPtr;

typedef std::pair<git_time_t, std::string> CommitId;
typedef std::map<CommitId, CommitPtr> CommitStorage;
typedef std::map<std::string, BranchPtr> BranchStorage;
typedef std::map<std::string, RepoPtr> RepoStorage;
typedef std::vector<std::string> RemoteList;

enum GitItemType
{
	GIT_REPO,
	GIT_REMOTE,
	GIT_COMMIT,
	GIT_REF,
	GIT_STR_ARR,
};

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Commit
{
public:
	Commit(const git_time& time = git_time(),
		   const std::string author = std::string(),
		   const std::string message = std::string());

	Commit(const Commit& other);
	Commit& operator=(const Commit& other);

	void setTime(const git_time& time);
	void setAuthor(const std::string& author);
	void setMessage(const std::string& message);

	git_time getTime() const;
	std::string getAuthor() const;
	std::string getMessage() const;

private:
	git_time mTime;
	std::string mAuthor;
	std::string mMessage;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Branch
{
public:
	Branch(const std::string branchName = std::string(),
		   const GitRefPtr branchRef = nullptr,
		   const bool& isRemote = false);

	Branch(const Branch& other);
	Branch& operator=(const Branch& other);

	void setBranchName(const std::string& branchName);
	void setRemote(const bool& isRemote);
	void setBranchRef(const GitRefPtr brancRef);
	void addCommit(const CommitPtr commit);	
	void clearCommits();

	const CommitStorage& getCommits() const;
	std::string getBranchName() const;
	GitRefPtr getBranchRef() const;
	bool isRemote() const;
	
private:
	std::string mBranchName;
	CommitStorage mCommits;
	GitRefPtr mBranchRef;
	bool mIsRemote;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                   Repo                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Repo
{
private:	
	typedef std::map<std::string, GitRemotePtr> Remotes;

public:
	Repo(const std::string& repoPath = std::string(),
		const GitRepoPtr repo = nullptr);

	Repo(const Repo& other);
	Repo& operator=(const Repo& other);
	
	/**<  repo operations */
	bool openLocal(const std::string& path);
	bool fetch();	
	void closeRepo();

	std::string getPath() const;
	bool isValid() const;		

	void print() const;

private:
	bool readRemoteList(std::set<std::string>& remotesList);	
	bool updateRemotes();
	bool updateBranches();
	bool updateBranchCommits(const BranchPtr branch);

	GitStrArrPtr createStrArr() const;
	std::string getCommitMessageStr(CommitPtr commit) const;

	//callbacks with params determined by the lib
	static int progress_cb(const char *str, int len, void *data);
	static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);

private:	
	GitRepoPtr mRepo;	
	Remotes mRemotes;

	std::string mUrl;
	std::string mPath;
	BranchStorage mLocalBranches;
	BranchStorage mRemoteBranches;
	
	std::vector<BranchPtr> mNewBranches;
	std::vector<CommitPtr> mNewCommits;
};



//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitHandler
{	
public:
	GitHandler();
	~GitHandler();

	bool registerItemTypes();
	bool addRepo(const RepoPtr repo);	

private:		
	int remoteLs(GitRemotePtr remote, std::vector<std::string>& refStor);
	int remoteDefBranch(GitRemotePtr remote, std::string& branchName);
	std::string remoteName(GitRemotePtr remote);
	
	int cloneRepo(GitRepoPtr repo, const std::string& url, const std::string& path, const git_clone_options& cloneOpts);
	
private:
	RepoStorage mRepos;
};

#endif // GITHANDLER_H
