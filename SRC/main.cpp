#include "githandler.h"
#include <iostream>

int main(int argc, char *argv[])
{			
	GitHandler h;	

	Repo r;
	//bool result = r.openLocal("D:\\Repo\\libgit2");
	bool result = r.openLocal("D:\\Test");
	result = r.fetch();

	r.printBranches();

	auto branches = r.getLocalBranches();
	auto branch = branches.begin()->first;

	r.printBranchCommits("test");

	std::cin.get();
/*
	GitHandler h;

	const std::string url = "https://github.com/Skoparov/TestRepo.git";
	const char* path = "D:\\Test";

	auto repo = h.openLocalRepo(path);*/	
}
