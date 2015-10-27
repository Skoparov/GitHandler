#ifndef GITHANDLER_H
#define GITHANDLER_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "GitItemFactory.h"

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

typedef std::shared_ptr<Commit>    CommitPtr;
typedef std::shared_ptr<Branch>    BranchPtr;
typedef std::shared_ptr<Repo>      RepoPtr;
typedef std::weak_ptr<Branch>      ParentBranchPtr;
typedef std::weak_ptr<Repo>        ParentRepoPtr;

typedef std::pair<std::string, uint64_t> CommitId;
typedef std::map<CommitId, CommitPtr> CommitStorage;
typedef std::map<std::string, BranchPtr> BranchStorage;
typedef std::map<std::string, RepoPtr> RepoStorage;

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
	explicit Commit(const git_time& time = git_time(),
		            const std::string author = std::string(),
		            const std::string message = std::string(),
		            const GitCommitPtr commitPtr = nullptr,
		            const BranchPtr parentBranch = nullptr);

	void setTime(const git_time& time);
	void setAuthor(const std::string& author);
	void setMessage(const std::string& message);
	void setGitCommit(const GitCommitPtr commitPtr);
	void setParentBranch(const BranchPtr parentBranch);

	git_time getTime() const;
	std::string getAuthor() const;
	std::string getMessage() const;
	GitCommitPtr getGitCommit() const;
	ParentBranchPtr getParentBranch() const;

private:
	git_time mTime;
	std::string mAuthor;
	std::string mMessage;
	GitCommitPtr mCommitPtr;
	ParentBranchPtr mParentBranch;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Branch
{
public:
	explicit Branch(const std::string branchName = std::string(),
		const GitRefPtr branchRef = nullptr,
		const bool& isRemote = false,
		const RepoPtr paretnRepo = nullptr);

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

	ParentRepoPtr mParentRepo;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                   Repo                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Repo
{
public:
	Repo(const std::string& repoPath = std::string(),
		 const std::string& repoUrl  = std::string(),
		 const GitRepoPtr repo = nullptr,
		 const GitRemotePtr remote = nullptr);
	
	/**<  repo operations */
	bool openLocalRepo();
	void closeRepo();

	/**< operations with branches */
	void addLocalBranch(const BranchPtr branch);
	void addRemoteBranch(const BranchPtr branch);
	void clearBranches();

	bool isValid() const;

	// setters
	void setUrl(const std::string& repoUrl);
	void setPath(const std::string& repoPath);	
	void setRepo(const GitRepoPtr repo);
	void setRemote(const GitRemotePtr remote);		
	
	// getters
	std::string getUrl() const;
	std::string getPath() const;
	GitRepoPtr getRepo() const;
	GitRemotePtr getRemote() const;
	const BranchStorage& getLocalBranches() const;
	const BranchStorage& getRemoteBranches() const;

private:
	bool getRemoteList(std::vector<std::string> remotes);

private:
	GitRepoPtr mRepo;
	GitRemotePtr mRemote;
	std::string mUrl;
	std::string mPath;
	std::vector<std::string> mOrigins;
	BranchStorage mLocalBranches;
	BranchStorage mRemoteBranches;
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
	std::string refName(GitRefPtr ref);
	git_oid refTarget(GitRefPtr ref);


	int cloneRepo(GitRepoPtr repo, const std::string& url, const std::string& path, const git_clone_options& cloneOpts);
	GitRemotePtr initRemote(GitRepoPtr repo, const std::string& name, const git_fetch_options* opts);
	int fetch(GitRemotePtr remote, const git_fetch_options& opts);

	int getBranchCommits(GitRefPtr branchRef, GitRepoPtr repo, std::vector<GitCommitPtr>& commits);
	int getBranches(GitRepoPtr repo);

	//callbacks with params determined by the lib
	static int progress_cb(const char *str, int len, void *data);
	static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data);

private:
	RepoStorage mRepos;
};

#endif // GITHANDLER_H
