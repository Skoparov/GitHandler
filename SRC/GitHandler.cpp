#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////                GitHandler               //////////////////////
//////////////////////////////////////////////////////////////////////////////

GitHandler::CurrentRepoPtr GitHandler::mCurrentRepo;
GitHandler::NewBranchStorage GitHandler::mNewBranches;
GitHandler::NewCommitStorage GitHandler::mNewCommits;

GitHandler::GitHandler()
{
	git_libgit2_init();	
	registerGitItemTypes();
}

bool GitHandler::addRepo(const RepoPtr repo)
{	
	if (repo->isValid())
	{
		mRepos.insert(std::make_pair(repo->path(), repo));
		return true;
	}

	return false;
}

bool GitHandler::update()
{
	bool result = true;

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks.update_tips = &update_cb;
	fetch_opts.callbacks.sideband_progress = &progress_cb;

	for (auto repo : mRepos)
	{
		mCurrentRepo = repo.second;
		result &= repo.second->fetch(fetch_opts);
	}

	return result;
}

void GitHandler::clear()
{
	mRepos.clear();
	mNewBranches.clear();
	mNewCommits.clear();
}

RepoPtr GitHandler::getRepo(const string& path) const
{	
	auto repoIter = mRepos.find(path);
	if(repoIter != mRepos.end()){
		return repoIter->second;
	}

	return nullptr;
}

const RepoStorage& GitHandler::getRepos() const
{
	return mRepos;
}

auto GitHandler::newBranches()->NewBranchStorage
{
	return NewBranchStorage(std::move(mNewBranches));
}

auto GitHandler::newCommits()->NewCommitStorage
{
	return NewCommitStorage(std::move(mNewCommits));
}

int GitHandler::progress_cb(const char *str, int len, void *data)
{
	//printf("remote: %.*s", len, str);
	//fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int GitHandler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
	if (mCurrentRepo.expired()){
		return 1;
	}

	RepoPtr currRepo = mCurrentRepo.lock();

	if (git_oid_iszero(oldHead)) // new branch
	{
		GitRefPtr ref = Aux::getReference(refname, currRepo->gitRepo());
		if (ref == nullptr){
			return 1;
		}

		BranchPtr branchPtr = std::make_shared<Branch>(ref, true);

		auto branchStor = mNewBranches.find(currRepo->path());
		if (branchStor == mNewBranches.end())
		{
			mNewBranches.insert(std::make_pair(currRepo->path(), vector<BranchPtr>()));
			branchStor = mNewBranches.find(currRepo->path());
		}

		branchStor->second.push_back(branchPtr);
	}
	else //new commit
	{				
		auto key = std::make_pair(currRepo->path(), refname);
		auto storage = mNewCommits.find(key);
		if (storage == mNewCommits.end())
		{
			mNewCommits.insert(std::make_pair(key, vector<CommitPtr>()));
			storage = mNewCommits.find(key);
		}

		auto branch = currRepo->getBranch(refname);
		if (branch == nullptr){
			return 1;
		}

		const CommitStorage& commits = branch->commits();
		auto oldHeadCommit = std::find_if(commits.begin(), commits.end(),
			                              [oldHead](CommitStorage::value_type commit)->bool
		                                  {return git_oid_equal(oldHead, &commit.second->id());});

		if (oldHeadCommit != commits.end()){
			oldHeadCommit++;
		}
			
		for (auto commitIt = oldHeadCommit; commitIt != commits.end(); ++commitIt){									
			storage->second.push_back(commitIt->second);
		}		
	}
	
	return 0;
}

bool GitHandler::registerGitItemTypes()
{
	bool ok = true;
	
	auto& c = GitItemCreator::get();

	ok &= c.registerItemType <git_repository, void(*)(git_repository*)> (GIT_REPO,     &deleteRepo);
	ok &= c.registerItemType <git_remote,     void(*)(git_remote*)>     (GIT_REMOTE,   &deleteRemote);
	ok &= c.registerItemType <git_commit,     void(*)(git_commit*)>     (GIT_COMMIT,   &deleteCommit);
	ok &= c.registerItemType <git_reference,  void(*)(git_reference*)>  (GIT_REF,      &deleteRef);
	ok &= c.registerItemType <git_strarray,   void(*)(git_strarray*)>   (GIT_STR_ARR,  &deleteStrArr);
	ok &= c.registerItemType <git_revwalk,    void(*)(git_revwalk*)>    (GIT_REV_WALK, &deleteRevWalk);

	return ok;
}

GitHandler::~GitHandler()
{
	clear();
	git_libgit2_shutdown();
}