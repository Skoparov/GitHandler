#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
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

void GitHandler::printNew() const
{
	for (auto newBranch = mNewBranches.begin(); newBranch != mNewBranches.end(); ++newBranch)
	{
		auto repo = getRepo(newBranch->first);
		if (repo != nullptr){
			printf("\nNew branch: \"%s\" | Repo: \"%s\"\n", newBranch->second->name().c_str(), repo->path().c_str());
		}
	}

	for (auto newCommit = mNewCommits.begin(); newCommit != mNewCommits.end(); ++newCommit)
	{
		auto repo = getRepo(newCommit->first);		
		if (repo != nullptr)
		{
			printf("\n");
			auto& commitData = newCommit->second;			
			printf("New commit | Repo \"%s\" | Branch \"%s\" \n", repo->path().c_str(), commitData.first.c_str());
			Aux::printCommit(commitData.second);
		}
	}
}

int GitHandler::progress_cb(const char *str, int len, void *data)
{
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int GitHandler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
	if (!mCurrentRepo.expired())
	{
		RepoPtr currRepo = mCurrentRepo.lock();

		if (git_oid_iszero(oldHead)) // new branch
		{
			GitRefPtr ref = Aux::getReference(refname, currRepo->gitRepo());
			if (ref != nullptr)
			{
				BranchPtr branch = std::make_shared<Branch>(ref, true);
				mNewBranches.insert(std::make_pair(currRepo->path(), branch));
			}
		}
		else //new commit
		{
			GitCommitPtr commit = Aux::readCommit(currRepo, head);
			if (commit != nullptr)
			{
				CommitPtr cPtr = std::make_shared<Commit>(commit);
				if (cPtr != nullptr)
				{
					string mess = cPtr->message();
					auto commitData = std::make_pair(refname, cPtr);
					mNewCommits.insert(std::make_pair(currRepo->path(), commitData));
				}
			}
		}
	}

	return 0;
}

bool GitHandler::registerGitItemTypes()
{
	bool ok = true;
	
	ok &= GitItemCreator::registerItemType <git_repository, void(*)(git_repository*)> (GIT_REPO,     &deleteRepo);
	ok &= GitItemCreator::registerItemType <git_remote,     void(*)(git_remote*)>     (GIT_REMOTE,   &deleteRemote);
	ok &= GitItemCreator::registerItemType <git_commit,     void(*)(git_commit*)>     (GIT_COMMIT,   &deleteCommit);
	ok &= GitItemCreator::registerItemType <git_reference,  void(*)(git_reference*)>  (GIT_REF,      &deleteRef);
	ok &= GitItemCreator::registerItemType <git_strarray,   void(*)(git_strarray*)>   (GIT_STR_ARR,  &deleteStrArr);
	ok &= GitItemCreator::registerItemType <git_revwalk,    void(*)(git_revwalk*)>    (GIT_REV_WALK, &deleteRevWalk);

	return ok;
}

GitHandler::~GitHandler()
{
	clear();
	git_libgit2_shutdown();
}