#include "GitUpdater.h"
#include <QApplication>

int main(int argc, char *argv[])
{		
	/*QApplication a(argc, argv);
	GitUpdater w;
	w.show();
	
	return a.exec();*/
	GitHandler h;	

	Repo r;
	bool result = r.openLocal("D:\\Test");
	result = r.fetch();
	r.print();

	int i = 0;

/*
	GitHandler h;

	const std::string url = "https://github.com/Skoparov/TestRepo.git";
	const char* path = "D:\\Test";

	auto repo = h.openLocalRepo(path);*/	
}
