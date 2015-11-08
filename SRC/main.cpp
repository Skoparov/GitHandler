#include "githandler.h"
#include <iostream>

void printNew(GitHandler& h)
{
	auto newBranches(std::move(h.newBranches()));
	for (auto newBranchesStor = newBranches.begin(); newBranchesStor != newBranches.end(); ++newBranchesStor)
	{		
		for (auto branch : newBranchesStor->second){
			printf("\nNew branch: \"%s\" | Repo: \"%s\"\n", branch->name().c_str(), newBranchesStor->first.c_str());
		}
	}

	auto newCommits(std::move(h.newCommits()));
	for (auto newCommit = newCommits.begin(); newCommit != newCommits.end(); ++newCommit)
	{		
		for (auto commit : newCommit->second)
		{
			printf("\nNew commit | Repo \"%s\" | Branch \"%s\" \n", newCommit->first.first.c_str(), newCommit->first.second.c_str());
			Aux::printCommit(commit);
		}	
	}
}

int main(int argc, char *argv[])
{			
	GitHandler h;
	string path = "D:\\Test";
	//string path = "D:\\Repo\\libgit2";	

			
	RepoPtr repo = std::make_shared<Repo>();
	bool result = repo->openLocal(path);
	h.addRepo(repo);
	
	h.update();	
	printNew(h);

	/*auto repoPtr = h.getRepo(path);
	if (repoPtr != nullptr)
	{
		BranchStorage locals;
		if (repoPtr->getBranches(locals, true))
		{
			Aux::printBranches(locals);
			for (auto branch : locals){
				Aux::printBranchCommits(branch.second);
			}
		}
	}*/
	
	std::cin.get();
}
