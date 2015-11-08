#include "githandler.h"
#include <iostream>

int main(int argc, char *argv[])
{			
	GitHandler h;
	string path = "D:\\Test";
	//string path = "D:\\Repo\\libgit2";	

			
	RepoPtr repo = std::make_shared<Repo>();
	bool result = repo->openLocal(path);
	h.addRepo(repo);

	int sad = repo.use_count();
	
	h.update();		

	/*auto repoPtr = h.getRepo(path);
	if (repoPtr != nullptr)
	{
		BranchStorage locals;
		if (repoPtr->getBranches(locals))
		{
			Aux::printBranches(locals);
			for (auto branch : locals){
				Aux::printBranchCommits(branch.second);
			}
		}
	}*/

	h.printNew();
	
	std::cin.get();
}
