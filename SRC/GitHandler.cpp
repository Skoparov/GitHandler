#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
//////////////////////////////////////////////////////////////////////////////

GitHandler::GitHandler()
{
	git_libgit2_init();	
	registerItemTypes();
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


GitHandler::~GitHandler()
{
	git_libgit2_shutdown();
}

//////////////////////////////////////////////////////////////////////////////
///////////////                 Commit                  //////////////////////
//////////////////////////////////////////////////////////////////////////////

Commit::Commit(const git_time& time,
	          const std::string author, 
	          const std::string message) : 

			  mTime(time),
              mAuthor(author), 
			  mMessage(message)
{

}

Commit::Commit(const Commit& other) : 
               mTime(other.mTime),
			   mAuthor(other.mAuthor),
			   mMessage(other.mMessage)
{

}

Commit& Commit::operator=(const Commit& other)
{
	if (&other == this){
		return *this;
	}

	mTime = other.mTime;
	mAuthor = other.mAuthor;
	mMessage = other.mMessage;	

	return *this;
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

std::string Commit::getAuthor() const
{
	return mAuthor;
}

std::string Commit::getMessage() const
{
	return mMessage;
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
			   const bool& isRemote) :

               mBranchName(branchName), 
			   mBranchRef(branchRef),
			   mIsRemote(isRemote)
{

}

Branch::Branch(const Branch& other) : 
               mBranchName(other.mBranchName),
			   mCommits(other.mCommits),
			   mBranchRef(other.mBranchRef),
			   mIsRemote(other.mIsRemote)
{

}

Branch& Branch::operator=(const Branch& other)
{	
	Branch b(other.mBranchName, other.mBranchRef, other.mIsRemote);

	if (&other == this){
		return *this;
	}

	mBranchName = other.mBranchName;
	mBranchRef = other.mBranchRef;
	mIsRemote = other.mIsRemote;	
	mCommits = other.mCommits;

	return *this;
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
	CommitId id(commit->getTime().time, commit->getMessage());

	if (!mCommits.count(id)){
		mCommits.insert(std::make_pair(id, commit));
	}
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


Repo::Repo(const std::string& repoPath, const GitRepoPtr repo) : 
           mPath(repoPath),         
           mRepo(repo)       
{

}

Repo::Repo(const Repo& other) : 
           mRepo(other.mRepo),
           mUrl(other.mUrl),
           mPath(other.mPath),                   
           mLocalBranches(other.mLocalBranches)
{

}

Repo& Repo::operator=(const Repo& other)
{
	if (&other == this){
		return *this;
	}
	
	mPath = other.mPath;
	mUrl = other.mUrl;
	mRepo = other.mRepo;	
	mLocalBranches = other.mLocalBranches;

	return *this;
}

bool Repo::openLocal(const std::string& path)
{
	closeRepo();
	mRepo = GitItemCreator::get().create<GitRepo>(GIT_REPO);
	
	if (mRepo == nullptr){
		return false;
	}
		
	git_repository* r;
	if (git_repository_open(&r, path.c_str()) != 0){
		return false;
	}

	mRepo->setItem(r);
	mPath = path;

	return true;
}

bool Repo::updateRemotes()
{		
	if (!isValid())
	{
		printf("Init remotes : Repo not initialized");
		return false;
	}
	
	std::set<std::string> remotesList;
	if (!readRemoteList(remotesList)){
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
			mRemotes.erase(remote++);
			printf("Init remotes : remote removed from the list: %s", remote->first.c_str());
		}
		else{
			++remote;
		}
	}

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks.update_tips = &update_cb;
	fetch_opts.callbacks.sideband_progress = &progress_cb;
		
	for (auto name : remotesList)
	{
		auto remoteStor = mRemotes.find(name);
		if (remoteStor == mRemotes.end())
		{
			git_remote* remote = nullptr;
			if (git_remote_lookup(&remote, mRepo->gitItem(), name.c_str()) != 0)
			{
				printf("Init remotes : git_remote_lookup failed for remote: %s", name);
				continue;
			}
		
			GitRemotePtr remotePtr = GitItemCreator::get().create<GitRemote>(GIT_REMOTE);
			if (remotePtr == nullptr)
			{
				printf("Init remotes : Failed to create storage for remote: %s", name);
				git_remote_free(remote);
				continue;
			}

			remotePtr->setItem(remote);

			if (git_remote_connect(remotePtr->gitItem(), GIT_DIRECTION_FETCH, &fetch_opts.callbacks, NULL) != 0)
			{
				printf("Init remotes : git_remote_connect failed for remote: %s", name);
				continue;
			}			

			mRemotes.insert(std::make_pair(name, remotePtr));
			printf("Init remotes : Remote added: %s\n", name.c_str());
		}		
	}

	return true;
}

bool Repo::fetch()
{	
	if (!isValid())
	{
		printf("Fetch : Repo not initialized");
		return false;
	}

	if (!updateRemotes()){
		return false;
	}

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	auto a = &Repo::update_cb;
	fetch_opts.callbacks.update_tips;
		//&update_cb; 
	fetch_opts.callbacks.sideband_progress = &this->progress_cb;

	for (auto& remote : mRemotes)
	{	
		std::string remoteName = remote.first;

		auto arr = createStrArr();
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

	if (!updateBranches()){
		return false;
	}

	return true;
}

bool Repo::readRemoteList(std::set<std::string>& remotesList)
{		
	if (!isValid())
	{
		printf("Read remotes list : Repo not initialized\n");
		return false;
	}

	auto remoteList = createStrArr();
	if (remoteList == nullptr)
	{
		printf("Read remotes list : Failed to create storage for remotes\n");
		return false;
	}

	if (git_remote_list(remoteList->gitItem(), mRepo->gitItem()) != 0)
	{
		printf("Read remotes list : git_remote_list failed\n");
		return false;
	}

	for (size_t strNum = 0; strNum < remoteList->item()->count; ++strNum){
		remotesList.insert(remoteList->item()->strings[strNum]);
	}
				
	return true;
}

bool Repo::updateBranches()
{	
	if (!isValid())
	{
		printf("Update branches  : Repo not initialized\n");
		return false;
	}

	auto arr = createStrArr();
	if (arr == nullptr)
	{
		printf("Update branches : Failed to create storage for branches\n");
		return false;
	}

	if (git_reference_list(arr->gitItem(), mRepo->gitItem()) != 0)
	{
		printf("Update branches : git_reference_list failed\n");
		return false;
	}

	for (size_t refNUm = 0; refNUm < arr->item()->count; ++refNUm)
	{
		std::string refName = arr->item()->strings[refNUm];
		if (mLocalBranches.count(refName) && mRemoteBranches.count(refName)){
			continue;
		}
				
		git_reference* ref = nullptr;
		if (git_reference_lookup(&ref, mRepo->gitItem(), refName.c_str()) != 0)
		{
			printf("Update branches : git_reference_lookup failed for ref: %s\n", refName.c_str());
			continue;
		}

		GitRefPtr refPtr = GitItemCreator::get().create<GitRef>(GIT_REF);
		if (refPtr == nullptr)
		{
			printf("Update branches : Failed to create GitRefPtr for ref: %s\n", refName.c_str());
			continue;
		}

		refPtr->setItem(ref);
		std::string branchName = git_reference_shorthand(refPtr->gitItem());

		if (git_reference_is_branch(refPtr->gitItem()))
		{					
			BranchPtr branch = std::make_shared<Branch>(branchName, refPtr, false);
			mLocalBranches.insert(std::make_pair(branchName, branch));
		}
		else if (git_reference_is_remote(refPtr->gitItem()))
		{
			BranchPtr branch = std::make_shared<Branch>(branchName, refPtr, true);
			mRemoteBranches.insert(std::make_pair(branchName, branch));
		}
	}

	for (const auto& branch : mLocalBranches){
		updateBranchCommits(branch.second);
	}

	for (const auto& branch : mRemoteBranches){
		updateBranchCommits(branch.second);
	}

	return 0;
}


bool Repo::updateBranchCommits(const BranchPtr branch)
{
	if (!isValid())
	{
		printf("Update branch commits  : Repo not initialized\n");
		return false;
	}
	
	git_oid* oid = const_cast<git_oid*>(git_reference_target(branch->getBranchRef()->gitItem()));
	git_revwalk *walker;	

	git_revwalk_new(&walker, mRepo->gitItem());
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker, oid);

	GitCommitPtr commitPtr = GitItemCreator::get().create<GitCommit>(GIT_COMMIT);
	if (commitPtr == nullptr)
	{
		printf("Update branch commits  : Failed to create commit\n");
		return false;
	}

	const CommitStorage& stor = branch->getCommits();

	while (git_revwalk_next(oid, walker) == 0)
	{
		git_commit* commit;
		if (git_commit_lookup(&commit, mRepo->gitItem(), oid))
		{
			printf("Update branch commits  : Failed to create commit\n");
			continue;
		}	
		
		commitPtr->setItem(commit);	

		const std::string message = git_commit_message(commitPtr->gitItem());
		const git_signature* authorSig = git_commit_author(commitPtr->gitItem());	

		CommitId id(authorSig->when.time, message);
		if (!stor.count(id))
		{
			CommitPtr cPtr = std::make_shared<Commit>(authorSig->when, authorSig->name, message);
			branch->addCommit(cPtr);
		}
	}

	git_revwalk_free(walker);

	return 0;
}

bool Repo::isValid() const
{
	return mRepo != nullptr &&
		   mRepo->isValid() &&
		   mPath.length();
}

void Repo::closeRepo()
{
	mRepo.reset();
	mPath = std::string();
	mUrl = std::string();
	mLocalBranches.clear();
	mRemotes.clear();
}

std::string Repo::getPath() const
{
	return mPath;
}

GitStrArrPtr Repo::createStrArr() const
{
	auto remoteList = GitItemCreator::get().create<GitStrArr>(GIT_STR_ARR);
	if (remoteList != nullptr)
	{
		remoteList->initItem();
		remoteList->item()->count = 0;
	}

	return remoteList;
}

int Repo::progress_cb(const char *str, int len, void *data)
{
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int Repo::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
	char a_str[GIT_OID_HEXSZ + 1], b_str[GIT_OID_HEXSZ + 1];

	git_oid_fmt(b_str, head);
	b_str[GIT_OID_HEXSZ] = '\0';
	
	if (git_oid_iszero(oldHead)) 
	{
		printf("[new]     %.20s %s\n", b_str, refname);

	/*	git_reference* ref = nullptr;
		if (git_reference_lookup(&ref, mRepo->gitItem(), refName.c_str()) != 0)
		{

		}

		GitRefPtr refPtr = GitItemCreator::get().create<GitRef>(GIT_REF);
		if (refPtr == nullptr)
		{

		}

		refPtr->setItem(ref);
		std::string branchName = git_reference_shorthand(refPtr->gitItem());
		BranchPtr branch = std::make_shared<Branch>(branchName, refPtr, true);*/
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

void Repo::print() const
{
	printf("\nBRANCHES\n\n");
	for (auto& branch : mRemoteBranches)
	{
		printf("-------------------\n");
		printf("%s\n", branch.first.c_str());
		printf("-------------------\n\n");

		auto commits = branch.second->getCommits();
		for (auto commit = commits.rbegin(); commit != commits.rend(); ++ commit)
		{
			QString rawMessage(commit->second->getMessage().c_str());
			QStringList l = rawMessage.split("\n", QString::SkipEmptyParts);
			QString message = l.join("\n");

			QDateTime dtime = QDateTime::fromMSecsSinceEpoch(commit->second->getTime().time * 1000);
			QString time = dtime.toString(Qt::ISODate).replace("T", " ");
			message.prepend(QString("[%1] %2\n").arg(time).arg(commit->second->getAuthor().c_str()));
					
			printf("%s\n\n", message.toStdString().c_str());
		}
	}

	for (auto& branch : mLocalBranches)
	{
		printf("-------------------\n");
		printf("%s\n", branch.first.c_str());
		printf("-------------------\n\n");

		auto commits = branch.second->getCommits();
		for (auto commit = commits.rbegin(); commit != commits.rend(); ++commit)
		{
			QString rawMessage(commit->second->getMessage().c_str());
			QStringList l = rawMessage.split("\n", QString::SkipEmptyParts);
			QString message = l.join("\n");

			QDateTime dtime = QDateTime::fromMSecsSinceEpoch(commit->second->getTime().time * 1000);
			QString time = dtime.toString(Qt::ISODate).replace("T", " ");
			message.prepend(QString("[%1] %2\n").arg(time).arg(commit->second->getAuthor().c_str()));

			printf("%s\n\n", message.toStdString().c_str());
		}
	}
}
