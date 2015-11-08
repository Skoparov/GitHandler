#ifndef GITBASECLASSES_H
#define GITBASECLASSES_H

#include <set>
#include <mutex>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "GitItemFactory.h"

using std::vector;
using std::string;

class Commit;
class Branch;
class Repo;
class Aux;

typedef std::shared_ptr<Commit>    CommitPtr;
typedef std::shared_ptr<Branch>    BranchPtr;
typedef std::shared_ptr<Repo>      RepoPtr;
typedef std::weak_ptr<Repo>        ParentRepoPtr;

typedef std::pair<git_time_t, std::string> CommitId;
typedef std::map<CommitId, CommitPtr> CommitStorage;
typedef std::map<string, BranchPtr> BranchStorage;
typedef std::set<string> RemotesList;

using namespace boost::posix_time;

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Commit
{
public:
	Commit(const GitCommitPtr commit = nullptr);

	Commit(const Commit& other);
	Commit& operator=(const Commit& other);	

	git_time time() const;
	string author() const;
	string message() const;
	GitCommitPtr commit() const;

	bool isValid() const;

private:	
	GitCommitPtr mCommit;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Branch
{
public:
	Branch(const GitRefPtr branchRef = nullptr,
		   const bool& isRemote = false,
		   const RepoPtr parentRepo = nullptr);

	Branch(const Branch& other);
	Branch& operator=(const Branch& other);

	void addCommit(const CommitPtr commit);	
	void clearCommits();
	
	string name() const;
	GitRefPtr ref() const;
	const CommitStorage& commits() const;	

	bool isRemote() const;
	bool isValid() const;
	
private:	
	CommitStorage mCommits;
	ParentRepoPtr mParentRepo;
	GitRefPtr mBranchRef;
	bool mIsRemote;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                   Repo                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Repo
{	
	typedef std::map<string, GitRemotePtr> Remotes;

public:
	Repo(const string& repoPath = string(),
		 const GitRepoPtr repo = nullptr);

	Repo(const Repo& other);
	Repo& operator=(const Repo& other);
	
	// repo operations
	bool openLocal(const string& path);
	bool fetch(const git_fetch_options& fetch_opts);
	bool clone(const string& url, const string& path, const git_clone_options& cloneOpts);
	void closeRepo();	

	// getters		
	string path() const;
	bool isValid() const;
	GitRepoPtr gitRepo() const;
	bool getBranches(BranchStorage& branchStor, bool getRemotes = false);

private:			
	bool readBranchCommits(const BranchPtr branch);
	bool readRemotesList(RemotesList& remotesList);
	bool updateRemotes(const git_fetch_options& fetch_opts);		

private:		
	GitRepoPtr mGitRepo;	
	Remotes mRemotes;
	string mLocalPath;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                   Aux                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

class Aux
{
public:	
	static GitStrArrPtr createStrArr();
	static string getCommitMessageStr(CommitPtr commit);	
	static GitRefPtr getReference(const string& refName, const GitRepoPtr repo);
	static GitCommitPtr readCommit(const RepoPtr repo, const git_oid *head);	
	static GitStrArrPtr getRepoRefList(const GitRepoPtr repo);	
	static string getBranchName(const string& fullBranchName);

	static void printBranches(const BranchStorage& storage);
	static void printCommit(const  CommitPtr commit);
	static void printBranchCommits(const BranchPtr branch);

private:
	std::mutex mMutex;
};

#endif // GITBASECLASSES_H
