#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				   GitHandler			    //////////////////////
//////////////////////////////////////////////////////////////////////////////

GitHandler::GitHandler()
{
	git_libgit2_init();
}

GitRepoPtr GitHandler::openRepo(const std::string& path)
{
	GitRepoPtr repoPtr = std::make_shared<GitRepo>(&git_repository_free);

	if (git_repository_open(repoPtr->getItemPointer(), path.c_str()) != 0){
		printf("git_repository_open failed\n");
	}

	return repoPtr;
}

int GitHandler::cloneRepo(GitRepoPtr repo, const std::string& url, const std::string& path, const git_clone_options& cloneOpts)
{	
	return git_clone(repo->getItemPointer(), url.c_str(), path.c_str(), &cloneOpts);
}


GitRemotePtr GitHandler::initRemote(GitRepoPtr repo, const std::string& name, const git_fetch_options* opts)
{
	GitRemotePtr remote = std::make_shared<GitRemote>(&git_remote_free);

	try
	{
		if (!repo || !repo->getItem()){
			throw std::exception("Repo is not initialized");
		}

		if (git_remote_lookup(remote->getItemPointer(), repo->getItem(), name.c_str()) != 0){
			throw std::exception("git_remote_lookup failed");
		}

		if (git_remote_connect(remote->getItem(), GIT_DIRECTION_FETCH, &opts->callbacks, NULL) != 0){
			throw std::exception("git_remote_connect failed");
		}
	}
	catch (const std::exception& e){
		printf("Error init remote : %s\n", e.what());		
	}

	return remote;
}

int GitHandler::remoteLs(GitRemotePtr remote, std::vector<std::string>& refStor)
{
	int result = -1;
	if (!remote || !remote->getItem()){
		return result;
	}	

	const git_remote_head **refs;
	size_t refsLen;

	result = git_remote_ls(&refs, &refsLen, remote->getItem());	
	for (size_t refNum = 0; refNum < refsLen && !result; refNum++)
	{
		char oid[GIT_OID_HEXSZ + 1] = { 0 };
		git_oid_fmt(oid, &refs[refNum]->oid);
		refStor.push_back(refs[refNum]->name);
	}

	return result;
}

int GitHandler::defaultBranch(GitRemotePtr remote, std::string& branchName)
{
	git_buf* buff = new git_buf();
	int result = git_remote_default_branch(buff, remote->getItem());
	if (!result){
		branchName = buff->ptr;
	}

	return result;
}

std::string GitHandler::remoteName(GitRemotePtr remote)
{	
	return std::string(git_remote_name(remote->getItem()));
}

int GitHandler::progress_cb(const char *str, int len, void *data)
{
	(void)data;
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int GitHandler::update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
{
	char a_str[GIT_OID_HEXSZ + 1], b_str[GIT_OID_HEXSZ + 1];
	(void)data;

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
		//printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);

	}

	return 0;
}

int GitHandler::fetch(GitRemotePtr remote, const git_fetch_options& opts)
{
	return git_remote_download(remote->getItem(), NULL, &opts);
}

std::vector<GitCommitPtr> GitHandler::getBranchCommits(GitRefPtr branchRef)
{
	std::vector<GitCommitPtr> stor;

	if (!branchRef || !branchRef->getItem() ){
		return stor;
	}		
	
	git_oid oid = refTarget(branchRef);	
	git_revwalk *walker;
	git_commit* commit;

	git_revwalk_new(&walker, mRepo);
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker, &oid);

	while (git_revwalk_next(&oid, walker) == 0)
	{
		if (git_commit_lookup(&commit, mRepo, &oid)){			
			continue;
		}

		stor.push_back(std::make_shared<GitCommit>(commit, &git_commit_free));

		//QString message(git_commit_message(commit));
		//const git_signature* sg = git_commit_author(commit);		
	}

	git_revwalk_free(walker);

	return stor;
}

std::string GitHandler::refName(GitRefPtr ref)
{
	if (!ref || !ref->getItem()){
		return std::string();
	}

	return git_reference_shorthand(ref->getItem());
}

git_oid GitHandler::refTarget(GitRefPtr ref)
{	
	if (!ref || !ref->getItem()){
		return git_oid();
	}

	return *git_reference_target(ref->getItem());
}
