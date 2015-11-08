#include "GitBaseClasses.h"

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Commit::Commit(const GitCommitPtr commit) :  mCommit(commit)
{

}

Commit::Commit(const Commit& other) : mCommit(other.mCommit)
{

}

Commit& Commit::operator=(const Commit& other)
{
	if (&other != this){
		mCommit = other.mCommit;
	}
	
	return *this;
}

string Commit::author() const
{
	string author;

	if (isValid()){
		author = git_commit_author(mCommit->gitItem())->name;
	}

	return author;
}

string Commit::message() const
{
	string message;

	if (isValid()){
		message = git_commit_message(mCommit->gitItem());
	}

	return message;
}

GitCommitPtr Commit::commit() const
{
	return mCommit;
}

git_time Commit::time() const
{
	git_time time;

	if (isValid()){
		time = git_commit_author(mCommit->gitItem())->when;
	}

	return time;
}

bool Commit::isValid() const
{
	return mCommit != nullptr && mCommit->item() != nullptr;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                Branch                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

Branch::Branch(const GitRefPtr branchRef, 
			   const bool& isRemote,
			   const RepoPtr parentRepo) :
              
			   mBranchRef(branchRef),
			   mIsRemote(isRemote),
			   mParentRepo(parentRepo)
{

}

Branch::Branch(const Branch& other) :               			  
			   mBranchRef(other.mBranchRef),
			   mIsRemote(other.mIsRemote),
			   mParentRepo(other.mParentRepo)
{

}

Branch& Branch::operator=(const Branch& other)
{		
	if (&other != this)
	{
		mBranchRef = other.mBranchRef;
		mIsRemote = other.mIsRemote;		
		mParentRepo = other.mParentRepo;
	}
		
	return *this;
}

void Branch::addCommit(const CommitPtr commit)
{
	CommitId id(commit->time().time, commit->message());

	if (!mCommits.count(id)){
		mCommits.insert(std::make_pair(id, commit));
	}
}

void Branch::clearCommits()
{
	mCommits.clear();
}

const CommitStorage& Branch::commits() const
{
	return mCommits;
}

string Branch::name() const
{
	string name; 

	if (isValid())
	{
		const char* c_name;
		if (git_branch_name(&c_name, mBranchRef->gitItem()) == 0){
			name = c_name;
		}
	}

	return name;
}

GitRefPtr Branch::ref() const
{
	return mBranchRef;
}

bool Branch::isRemote() const
{
	return mIsRemote;
}

bool Branch::isValid() const
{
	return mBranchRef != nullptr && mBranchRef->item() != nullptr;
}

//////////////////////////////////////////////////////////////////////////////
///////////////                   Repo                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Repo::Repo(const string& repoPath, const GitRepoPtr repo) : 
           mLocalPath(repoPath),         
           mGitRepo(repo)       
{

}

Repo::Repo(const Repo& other) : 
           mGitRepo(other.mGitRepo),        
           mLocalPath(other.mLocalPath)                          
{

}

Repo& Repo::operator=(const Repo& other)
{
	if (&other == this){
		return *this;
	}
	
	mLocalPath = other.mLocalPath;
	mGitRepo = other.mGitRepo;		

	return *this;
}

bool Repo::openLocal(const string& path)
{
	closeRepo();

	mGitRepo = GitItemCreator::create<GitRepo>(GIT_REPO);	
	if (mGitRepo == nullptr){
		return false;
	}
		
	git_repository* r;
	if (git_repository_open(&r, path.c_str()) != 0){
		return false;
	}

	mGitRepo->setItem(r);
	mLocalPath = path;

	return true;
}

bool Repo::updateRemotes(const git_fetch_options& fetch_opts)
{		
	if (!isValid()){		
		return false;
	}
	
	RemotesList remotesList;
	if (!readRemotesList(remotesList)){
		return false;
	}

	if (!remotesList.size())
	{
		printf("Init remotes : not remotes specified");
		return true;
	}

	for (auto remote = mRemotes.begin(); remote != mRemotes.end();)
	{
		if (!remotesList.count(remote->first))
		{
			mRemotes.erase(remote);
			printf("Init remotes : remote removed from the list: %s", remote->first.c_str());
		}
		
		remote++;		
	}		
		
	for (auto name : remotesList)
	{
		auto remoteStor = mRemotes.find(name);
		if (remoteStor == mRemotes.end())
		{
			git_remote* remote = nullptr;
			if (git_remote_lookup(&remote, mGitRepo->gitItem(), name.c_str()) != 0){				
				continue;
			}
		
			GitRemotePtr remotePtr = GitItemCreator::create<GitRemote>(GIT_REMOTE);
			if (remotePtr == nullptr)
			{				
				git_remote_free(remote);
				continue;
			}

			remotePtr->setItem(remote);

			if (git_remote_connect(remotePtr->gitItem(), GIT_DIRECTION_FETCH, &fetch_opts.callbacks, NULL) != 0){				
				continue;
			}			

			mRemotes.insert(std::make_pair(name, remotePtr));
			printf("Init remotes : Remote added: %s\n", name.c_str());
		}		
	}

	return true;
}

bool Repo::fetch(const git_fetch_options& fetch_opts)
{	
	if (!isValid())
	{
		printf("Fetch : Repo not initialized");
		return false;
	}
	
	if (!updateRemotes(fetch_opts)){
		return false;
	}	

	for (auto& remote : mRemotes)
	{
		string remoteName = remote.first;
		printf("Fetching : %s\n", remoteName.c_str());

		auto arr = Aux::createStrArr();
		if (arr == nullptr)
		{
			printf("Fetch : Failed to create storage for remote: %s", remoteName.c_str());
			continue;
		}

		if (git_remote_fetch(remote.second->gitItem(), arr->gitItem(), &fetch_opts, NULL) != 0)
		{
			printf("Fetch : git_remote_fetch failed for remote: %s", remoteName.c_str());
			continue;
		}
	}

	return true;
}

bool Repo::clone(const string& url, const string& path, const git_clone_options& cloneOpts)
{
	closeRepo();

	GitRepoPtr repo;
	bool result = false;

	git_repository* r = repo->item().get();
	if (git_clone(&r, url.c_str(), path.c_str(), &cloneOpts) == 0)
	{
		repo->setItem(r);
		result = true;
	}

	return result;
}

bool Repo::readRemotesList(RemotesList& remotesList)
{		
	if (!isValid())
	{
		printf("Read remotes list : Repo not initialized\n");
		return false;
	}

	auto remoteList = Aux::createStrArr();
	if (remoteList == nullptr)
	{
		printf("Read remotes list : Failed to create storage for remotes\n");
		return false;
	}

	if (git_remote_list(remoteList->gitItem(), mGitRepo->gitItem()) != 0)
	{
		printf("Read remotes list : git_remote_list failed\n");
		return false;
	}

	for (size_t strNum = 0; strNum < remoteList->item()->count; ++strNum){
		remotesList.insert(remoteList->item()->strings[strNum]);
	}
				
	return true;
}

bool Repo::readBranchCommits(const BranchPtr branch)
{	
	if (!isValid()){				
		return false;
	}
	
	git_oid* oid = const_cast<git_oid*>(git_reference_target(branch->ref()->gitItem()));
	if (oid == nullptr){
		return false;
	}
	
	GitRevWalkPtr walker;
	git_revwalk *gitWalker;

	if (git_revwalk_new(&gitWalker, mGitRepo->gitItem()) != 0){
		return false;
	}
	
	walker = GitItemCreator::create<GitRevWalk>(GIT_REV_WALK);
	if (walker == nullptr){
		return false;
	}

	walker->setItem(gitWalker);
	git_revwalk_sorting(walker->gitItem(), GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker->gitItem(), oid);
		
	GitCommitPtr commitPtr = GitItemCreator::create<GitCommit>(GIT_COMMIT);
	if (commitPtr == nullptr){		
		return false;
	}

	bool result = true;
	const CommitStorage& stor = branch->commits();

	while (git_revwalk_next(oid, walker->gitItem()) == 0)
	{
		git_commit* commit;
		if (git_commit_lookup(&commit, mGitRepo->gitItem(), oid) != 0)
		{			
			result = false;
			break;
		}	
		
		commitPtr->setItem(commit);	

		CommitPtr cPtr = std::make_shared<Commit>(commitPtr);
		if (cPtr == nullptr)
		{
			result = false;
			break;
		}

		branch->addCommit(cPtr);
	}

	if (!result){
		branch->clearCommits();
	}

	return result;
}

void Repo::closeRepo()
{
	mGitRepo.reset();
	mLocalPath = string();
	mRemotes.clear();
}

bool Repo::getBranches(BranchStorage& branchStorage, bool getRemotes /*= false*/)
{
	if (!isValid())
	{
		printf("Get local branches: Repo not initialized\n");
		return false;
	}

	auto arr = Aux::getRepoRefList(mGitRepo);
	if (arr == nullptr)
	{
		printf("Get local branches: Failed to get repo's refs list\n");
		return false;
	}

	for (size_t refNum = 0; refNum < arr->item()->count; ++refNum)
	{
		string refName = arr->item()->strings[refNum];

		GitRefPtr refPtr = Aux::getReference(refName, mGitRepo);
		if (refPtr == nullptr)
		{
			printf("Get local branches: Failed to create GitRefPtr for ref: %s\n", refName.c_str());
			continue;
		}

		int (*checkFunc)(const git_reference *) = getRemotes?
		                 git_reference_is_remote : git_reference_is_branch;

		if (checkFunc(refPtr->gitItem()))
		{
			BranchPtr branch = std::make_shared<Branch>(refPtr, true);
			branchStorage.insert(std::make_pair(branch->name(), branch));
		}		
	}

	for (const auto& branch : branchStorage)
	{
		printf("Reading branch commits for branch: %s\r", branch.first.c_str());
		readBranchCommits(branch.second);
	}

	printf("\n");

	return true;
}

GitRepoPtr Repo::gitRepo() const
{
	return mGitRepo;
}

string Repo::path() const
{
	return mLocalPath;
}

bool Repo::isValid() const
{
	return mGitRepo != nullptr &&
		mGitRepo->isValid() &&
		mLocalPath.length();
}

//////////////////////////////////////////////////////////////////////////////
///////////////                   Aux                   //////////////////////
//////////////////////////////////////////////////////////////////////////////

GitStrArrPtr Aux::createStrArr()
{
	auto remoteList = GitItemCreator::create<GitStrArr>(GIT_STR_ARR);
	if (remoteList != nullptr)
	{
		remoteList->initItem();
		remoteList->item()->count = 0;
	}

	return remoteList;
}

string Aux::getCommitMessageStr(CommitPtr commit)
{
	if (commit == nullptr){
		return string();
	}

	string rawMessage = commit->message();
	vector<string> parts;
	boost::split(parts, rawMessage, boost::is_any_of("\n"));
	rawMessage = boost::join_if(parts,
		                        "\n",
		                        [](const string& str)->bool
	                            {return str.length() != 0; });


	auto commitTime = commit->time();
	time_duration td(0, 0, 0, time_duration::ticks_per_second() * (commitTime.time + commitTime.offset * 60));
	ptime dtime = ptime(boost::gregorian::date(1970, 1, 1), td);

	string message = (boost::format("[%s]\n%s\n%s\n\n")
		% dtime
		% rawMessage
		% commit->author()).str();

	return message;
}

GitRefPtr Aux::getReference(const string& refName, const GitRepoPtr repo)
{
	GitRefPtr refPtr;
	git_reference* ref = nullptr;
	if (git_reference_lookup(&ref, repo->gitItem(), refName.c_str()) == 0)
	{
		refPtr = GitItemCreator::create<GitRef>(GIT_REF);
		if (refPtr != nullptr){
			refPtr->setItem(ref);
		}
	}

	return refPtr;
}

GitCommitPtr Aux::readCommit(const RepoPtr repo, const git_oid *head)
{
	GitCommitPtr commit;
	git_commit* gitCommit;

	if (head != nullptr && 
		git_commit_lookup(&gitCommit, repo->gitRepo()->gitItem(), head) == 0)
	{
		commit = GitItemCreator::create<GitCommit>(GIT_COMMIT);
		commit->setItem(gitCommit);		
	}

	return commit;
}

GitStrArrPtr Aux::getRepoRefList(const GitRepoPtr repo)
{
	GitStrArrPtr result;

	if (repo != nullptr && repo->isValid())
	{
		result = Aux::createStrArr();

		if (result == nullptr || 
			git_reference_list(result->gitItem(), repo->gitItem()) != 0)
		{
			result.reset();
		}		
	}	

	return result;
}

string Aux::getBranchName(const string& fullBranchName)
{
	return fullBranchName;
}

void Aux::printBranches(const BranchStorage& storage)
{	
	printf("\n-------------------\n");
	printf("%s\n", "BRANCHES");
	printf("-------------------\n\n");
	
	for (auto& branch : storage){
		printf("%s\n", branch.first.c_str());
	}
}

void Aux::printCommit(const CommitPtr commit)
{
	string message = Aux::getCommitMessageStr(commit);
	printf("%s", message.c_str());
}

void Aux::printBranchCommits(const BranchPtr branch)
{	
	if (branch != nullptr)
	{
		printf("\n-------------------\n");
		printf("%s\n", branch->name().c_str());
		printf("-------------------\n\n");

		auto commits = branch->commits();
		for (auto& commit : commits){
			printCommit(commit.second);
		}
	}
}
