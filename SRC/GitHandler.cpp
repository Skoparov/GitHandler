#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
//////////////////////////////////////////////////////////////////////////////

GitHandler::GitHandler()
{
	git_libgit2_init();	

	registerItemTypes();
}

bool GitHandler::registerItemTypes()
{
	bool ok = true;
	GitItemCreator& c = GitItemCreator::get();

	ok &= c.registerItemType <git_repository, void(*)(git_repository*)>(GIT_REPO, &deleteRepo);
	ok &= c.registerItemType <git_remote, void(*)(git_remote*)>(GIT_REMOTE, &deleteRemote);
	ok &= c.registerItemType <git_commit, void(*)(git_commit*)>(GIT_COMMIT, &deleteCommit);
	ok &= c.registerItemType <git_reference, void(*)(git_reference*)>(GIT_REF, &deleteRef);
	ok &= c.registerItemType <git_strarray, void(*)(git_strarray*)>(GIT_STR_ARR, &deleteStrArr);

	return ok;
}

bool GitHandler::addRepo(const RepoPtr repo)
{
	if (repo->isValid())
	{
		mRepos.insert(std::make_pair(repo->getPath(), repo));
		return true;
	}

	return false;
}

int GitHandler::cloneRepo(GitRepoPtr repo, const std::string& url, const std::string& path, const git_clone_options& cloneOpts)
{	
	git_repository* r = repo->item().get();
	int result = git_clone(&r, url.c_str(), path.c_str(), &cloneOpts);
	repo->setItem(r);

	return result;
}

GitRemotePtr GitHandler::initRemote(GitRepoPtr repo, const std::string& name, const git_fetch_options* opts)
{
	GitRemotePtr remote = GitItemCreator::get().create<GitRemote>(GIT_REMOTE);		

	try
	{
		if (!repo || !repo->isValid()){
			throw std::exception("Repo is not initialized");
		}

		git_remote* r = remote->item().get();
		if (git_remote_lookup(&r, repo->item().get(), name.c_str()) != 0){
			throw std::exception("git_remote_lookup failed");
		}

		remote->setItem(r);

		if (git_remote_connect(remote->item().get(), GIT_DIRECTION_FETCH, &opts->callbacks, NULL) != 0){
			throw std::exception("git_remote_connect failed");
		}
	}
	catch (const std::exception& e)
	{
		printf("Error init remote : %s\n", e.what());	
		remote.reset();
	}

	return remote;
}

int GitHandler::remoteLs(GitRemotePtr remote, std::vector<std::string>& refStor)
{
	int result = -1;
	if (!remote || !remote->item()){
		return result;
	}	

	const git_remote_head **refs;
	size_t refsLen = 0;

	result = git_remote_ls(&refs, &refsLen, remote->item().get());

	for (size_t refNum = 0; refNum < refsLen && !result; refNum++)
	{
		char oid[GIT_OID_HEXSZ + 1] = { 0 };
		git_oid_fmt(oid, &refs[refNum]->oid);
		refStor.push_back(refs[refNum]->name);
	}

	if (result){
		refStor.clear();
	}

	return result;
}

int GitHandler::remoteDefBranch(GitRemotePtr remote, std::string& branchName)
{
	int result = -1;
	if (!remote || !remote->item()){
		return result;
	}

	std::unique_ptr<git_buf> buff = std::make_unique<git_buf>();
	result = git_remote_default_branch(buff.get(), remote->item().get());

	if (!result){
		branchName = buff->ptr;
	}

	return result;
}

std::string GitHandler::remoteName(GitRemotePtr remote)
{	
	return std::string(git_remote_name(remote->item().get()));
}

int GitHandler::progress_cb(const char *str, int len, void *data)
{	
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int GitHandler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
	char a_str[GIT_OID_HEXSZ + 1], b_str[GIT_OID_HEXSZ + 1];

	git_oid_fmt(b_str, head);
	b_str[GIT_OID_HEXSZ] = '\0';

	if (git_oid_iszero(oldHead)) {
		printf("[new]     %.20s %s\n", b_str, refname);
	}
	else
	{
		/*git_commit* gcommit = nullptr;
		const git_signature* sg = nullptr;
		const char* message;
		const char* author;

		int res = git_commit_lookup(&gcommit, repo, oldHead);
		if (res == 0)
		{
			message = git_commit_message(gcommit);
			sg = git_commit_author(gcommit);
			author = sg->name;

			printf("Old head : %s | by %s in %s\n--------------------------- \n", message, author, refname);

			git_commit_free(gcommit);
		}

		res = git_commit_lookup(&gcommit, repo, head);
		if (res == 0)
		{
		message = git_commit_message(gcommit);
		sg = git_commit_author(gcommit);
		author = sg->name;

		printf("Head commit : %s | by %s in %s\n--------------------------- \n", message, author, refname);

		git_commit_free(gcommit);
		}*/

		git_oid_fmt(a_str, oldHead);
		a_str[GIT_OID_HEXSZ] = '\0';
		printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
	}

	return 0;
}

int GitHandler::fetch(GitRemotePtr remote, const git_fetch_options& opts)
{
	return git_remote_download(remote->item().get(), NULL, &opts);
}

int GitHandler::getBranches(GitRepoPtr repo)
{
	int result = -1;

	if (!repo ||!repo->item()){
		return result;
	}

	git_strarray* arr = new git_strarray;
	int res = git_reference_list(arr, repo->item().get());

	std::vector<git_reference*> branches;
	std::vector<git_reference*> remote_branches;

	for (int i = 0; i < arr->count; ++i)
	{
		char* refName = arr->strings[i];
		git_reference* ref = nullptr;

		res = git_reference_lookup(&ref, repo->item().get(), refName);
		if (git_reference_is_branch(ref))
		{
			printf("Branch : %s\n", git_reference_shorthand(ref));
			branches.push_back(ref);
		}
		else if (git_reference_is_remote(ref))
		{
			printf("Remote : %s\n", git_reference_shorthand(ref));
			remote_branches.push_back(ref);
		}
	}

	return 0;
}

int GitHandler::getBranchCommits(GitRefPtr branchRef, GitRepoPtr repo, std::vector<GitCommitPtr>& commits)
{
	int result = -1;

	if (!repo              || 
		!repo->isValid()   ||
		!branchRef         || 
		!branchRef->isValid())
	{
		return result;
	}		
	
	git_oid oid = refTarget(branchRef);	
	git_revwalk *walker;
	git_commit* commit;	

	git_revwalk_new(&walker, repo->item().get());
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker, &oid);

	while (git_revwalk_next(&oid, walker) == 0)
	{
		if (git_commit_lookup(&commit, repo->item().get(), &oid)){			
			continue;
		}

		auto commitPtr = GitItemCreator::get().create<GitCommit>(GIT_COMMIT);
		commits.push_back(commitPtr);

		//QString message(git_commit_message(commit));
		//const git_signature* sg = git_commit_author(commit);		
	}

	git_revwalk_free(walker);

	return 0;
}

std::string GitHandler::refName(GitRefPtr ref)
{
	if (!ref || !ref->isValid()){
		return std::string();
	}

	return git_reference_shorthand(ref->item().get());
}

git_oid GitHandler::refTarget(GitRefPtr ref)
{	
	if (!ref || !ref->isValid()){
		return git_oid();
	}

	const git_oid* result = git_reference_target(ref->item().get());
	return result != nullptr ? *result : git_oid();
}

GitHandler::~GitHandler()
{
	git_libgit2_shutdown();
}

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Commit::Commit(const git_time& time,
	          const std::string author, 
	          const std::string message, 
			  const GitCommitPtr commitPtr,
			  const BranchPtr parentBranch) : 

			  mTime(time),
              mAuthor(author), 
			  mMessage(message), 
			  mCommitPtr(commitPtr),
			  mParentBranch(parentBranch)
{

}

void Commit::setTime(const git_time& time)
{
	mTime = time;
}

void Commit::setAuthor(const std::string& author)
{
	mAuthor = author;
}

void Commit::setMessage(const std::string& message)
{
	mMessage = message;
}

void Commit::setGitCommit(const GitCommitPtr commitPtr)
{
	mCommitPtr = commitPtr;
}

void Commit::setParentBranch(const BranchPtr parentBranch)
{
	mParentBranch = parentBranch;
}

std::string Commit::getAuthor() const
{
	return mAuthor;
}

std::string Commit::getMessage() const
{
	return mMessage;
}

GitCommitPtr Commit::getGitCommit() const
{
	return mCommitPtr;
}

ParentBranchPtr Commit::getParentBranch() const
{
	return mParentBranch;
}

git_time Commit::getTime() const
{
	return mTime;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

Branch::Branch(const std::string branchName, 
	           const GitRefPtr branchRef, 
			   const bool& isRemote, 
			   const RepoPtr paretnRepo) :

               mBranchName(branchName), 
			   mBranchRef(branchRef),
			   mIsRemote(isRemote)
{

}

void Branch::setBranchName(const std::string& branchName)
{
	mBranchName = branchName;
}

void Branch::setRemote(const bool& isRemote)
{
	mIsRemote = isRemote;
}

void Branch::addCommit(const CommitPtr commit)
{
	CommitId id(commit->getMessage(), commit->getTime().time);
	mCommits.insert(std::make_pair(id, commit));
}

void Branch::clearCommits()
{
	mCommits.clear();
}

const CommitStorage& Branch::getCommits() const
{
	return mCommits;
}

std::string Branch::getBranchName() const
{
	return mBranchName;
}

GitRefPtr Branch::getBranchRef() const
{
	return mBranchRef;
}

bool Branch::isRemote() const
{
	return mIsRemote;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                   Repo                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Repo::Repo(const std::string& repoPath, 
	       const std::string& repoUrl, 
		   const GitRepoPtr repo, 
		   const GitRemotePtr remote) : 

           mPath(repoPath),
           mUrl(repoUrl),
           mRepo(repo),
		   mRemote(remote)
{

}

bool Repo::openLocalRepo()
{
	bool result = true;

	mRepo = GitItemCreator::get().create<GitRepo>(GIT_REPO);		

	if (mRepo == nullptr){
		return false;
	}	

	auto repoPtr = mRepo->item().get();

	if (git_repository_open(&repoPtr, mPath.c_str()) != 0)
	{
		printf("git_repository_open failed\n");
		mRepo.reset();
		result = false;
	}

	mRepo->setItem(repoPtr);

	std::vector<std::string> d;
	bool ok = getRemoteList(d);

	return result;
}

void Repo::closeRepo()
{
	mRepo.reset();
	mPath = std::string();
	mUrl = std::string();
	clearBranches();
}

void Repo::clearBranches()
{
	mLocalBranches.clear();
	mRemoteBranches.clear();
}

bool Repo::getRemoteList(std::vector<std::string> remotes)
{	
	auto remoteList = GitItemCreator::get().create<GitStrArr>(GIT_STR_ARR);

	if (!isValid() || !remoteList){
		return false;
	}
	
	std::shared_ptr<git_strarray> remoteList1;

	{		
		remoteList1 = std::shared_ptr<git_strarray>(new git_strarray(), deleteStrArr);

		if (git_remote_list(remoteList1.get(), mRepo->item().get()) != 0){
			return false;
		}

		for (size_t strNum = 0; strNum < remoteList1->count; ++strNum){
			mOrigins.push_back(remoteList1->strings[strNum]);
		}
	}
				
	return true;
}

void Repo::setUrl(const std::string& repoUrl)
{
	mUrl = repoUrl;
}

void Repo::setPath(const std::string& repoPath)
{
	mPath = repoPath;
}

void Repo::setRepo(const GitRepoPtr repo)
{
	mRepo = repo;
}

void Repo::addLocalBranch(const BranchPtr branch)
{
	mLocalBranches.insert(std::make_pair(branch->getBranchName(), branch));
}

void Repo::addRemoteBranch(const BranchPtr branch)
{
	mRemoteBranches.insert(std::make_pair(branch->getBranchName(), branch));
}

std::string Repo::getUrl() const
{
	return mUrl;
}

std::string Repo::getPath() const
{
	return mPath;
}

GitRepoPtr Repo::getRepo() const
{
	return mRepo;
}

void Repo::setRemote(const GitRemotePtr remote)
{
	mRemote = remote;
}

const BranchStorage& Repo::getLocalBranches() const
{
	return mLocalBranches;
}

const BranchStorage& Repo::getRemoteBranches() const
{
	return mRemoteBranches;
}

GitRemotePtr Repo::getRemote() const
{
	return mRemote;
}

bool Repo::isValid() const
{
	return mRepo != nullptr &&
		   mRepo->isValid() &&
		   mPath.length();
}
