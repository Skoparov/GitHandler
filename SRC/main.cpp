#include "gitupdater.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <QString>
#include <QApplication>

#include "include/git2.h"

int COMPLETE = 0;

git_repository* openRepo(const char* path)
{
	git_repository* repo = nullptr;

	if (git_repository_open(&repo, path) != 0){
		printf("git_repository_open failed\n");
	}

	return repo;
}

int cloneRepo(git_repository* repo, const char* url, const char* path)
{		
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

	int result = git_clone(&repo, url, path, &clone_opts);

	return result;
}

git_remote* initRemote(const char* name, git_repository* repo, const git_fetch_options* opts)
{
	git_remote* remote = nullptr;

	try
	{
		if (!repo){
			throw std::exception("Repo = null");
		}

		if (git_remote_lookup(&remote, repo, name) != 0){
			throw std::exception("git_remote_lookup failed");
		}

		if (git_remote_connect(remote, GIT_DIRECTION_FETCH, &opts->callbacks, NULL) != 0){
			throw std::exception("git_remote_connect failed");
		}
	}
	catch (const std::exception& e)
	{
		printf("Error init remote : %s\n", e.what());
		git_remote_free(remote);
		remote = nullptr;
	}

	return remote;
}

int remoteLs(git_remote* remote)
{
	const git_remote_head **refs;
	size_t refs_len;

	int result = git_remote_ls(&refs, &refs_len, remote);

	if (!result)
	{
		for (int i = 0; i < refs_len; i++)
		{
			char oid[GIT_OID_HEXSZ + 1] = { 0 };
			git_oid_fmt(oid, &refs[i]->oid);
			char* name = refs[i]->name;
			printf("%s\t%s\n", oid, refs[i]->name);
		}
	}
	
	return result;
}

int defBranch(git_remote* remote)
{
	git_buf* buff = new git_buf();
	int result = git_remote_default_branch(buff, remote);

	printf("DefBranch : %s\n", buff->ptr);

	return result;
}

void remoteName(git_remote* remote)
{
	const char* name = git_remote_name(remote);
	if (name){
		printf("Name : %s\n", name);
	}	
}

static int progress_cb(const char *str, int len, void *data)
{
	(void)data;
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

git_repository *repo;

static int update_cb(const char *refname, const git_oid *oldHead, const git_oid *head, void *data)
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
		git_commit* gcommit = nullptr;	
		const git_signature* sg = nullptr;
		const char* message;
		const char* author;

		/*int res = git_commit_lookup(&gcommit, repo, oldHead);
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

int fetch(git_remote* remote, const git_fetch_options& opts)
{
	return git_remote_download(remote, NULL, &opts);
}

void printBranchCommits(git_reference* branch_ref)
{
	const char* branchName = git_reference_shorthand(branch_ref);
	printf("BRANCH : %s \n", branchName);

	const git_oid* pOid = git_reference_target(branch_ref);
	git_oid oid = *pOid;

	git_revwalk *walker;
	git_commit* commit;

	git_revwalk_new(&walker, repo);
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker, &oid);

	while (git_revwalk_next(&oid, walker) == 0)
	{
		if (git_commit_lookup(&commit, repo, &oid))
		{
			fprintf(stderr, "Failed to lookup the next object\n");
			//fix mem leaks
			continue;
		}

		QString message(git_commit_message(commit));
		const git_signature* sg = git_commit_author(commit);
		QString author(sg->name);

		message.replace('\n', ' ');

		QString text = QString("|-> %1 | by %2\n").arg(message).arg(author);
		std::cout << text.toStdString();
	}

	printf("\n\n");
}

int main(int argc, char *argv[])
{
	git_libgit2_init();
	
	const char* url = "https://github.com/Skoparov/TestRepo.git";
	const char* path = "D:\\Test";
	int result = 0;

	//clone
	//cloneRepo(repo, url, path);

	//open
	repo = openRepo(path);
		
	git_strarray* arr = new git_strarray;
	int res = git_reference_list(arr, repo);

	std::vector<git_reference*> branches;
	std::vector<git_reference*> remote_branches;

	for (int i = 0; i < arr->count; ++i)
	{		
		char* refName = arr->strings[i];
		git_reference* ref = nullptr;
	
		res = git_reference_lookup(&ref, repo, refName);
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
	
	std::cout << "\n\n";

	git_revwalk* walker = nullptr;
	git_commit *commit = nullptr;
	
	for (auto branch_ref : branches){
		printBranchCommits(branch_ref);
	}

	for (auto branch_ref : remote_branches){
		printBranchCommits(branch_ref);
	}

	std::cin.get();

/*
	git_revwalk_new(&walker, repo);
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
	git_revwalk_push(walker, &oid);*/

	/*for (; !git_revwalk_next(&oid, walker); git_commit_free(commit)) 
	{
		
		const  char* message = git_commit_message(commit);
		const git_signature* sg = git_commit_author(commit);
		char* author = sg->name;

		printf("Commit : %s | by %s\n--------------------------- \n", message, author);				
	}*/

	return 0;

	//work with remote
	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks.update_tips = &update_cb;
	fetch_opts.callbacks.sideband_progress = &progress_cb;

	git_remote* remote = initRemote("origin", repo, &fetch_opts);
				
	if (remote)
	{
		//defBranch(remote);
		remoteLs(remote);
		//remoteName(remote);

		//		
	
		//int result = fetch(remote, fetch_opts);
		git_strarray* arr = new git_strarray;
		arr->count = 0;
		int result = git_remote_fetch(remote, arr, &fetch_opts, NULL);
		delete arr;

		git_remote_disconnect(remote);
		//result = git_remote_update_tips(remote);
		git_remote_free(remote);
	}


	if (repo){
		git_repository_free(repo);
	}

	git_libgit2_shutdown();

	std::cin.get();
		
	
	/*QApplication a(argc, argv);
	GitUpdater w;
	w.show();
	
	return a.exec();*/
}
