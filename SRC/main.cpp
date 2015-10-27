#include "GitUpdater.h"
#include <QApplication>


class testt
{
public:
	~testt()
	{
		int i = 0;
	}
};

void deleteTest(testt* i)
{
	delete i;
}

int main(int argc, char *argv[])
{		
	/*QApplication a(argc, argv);
	GitUpdater w;
	w.show();
	
	return a.exec();*/
	GitHandler h;

	Repo r("D:\\Test");
	bool result = r.openLocalRepo();

/*
	GitHandler h;

	const std::string url = "https://github.com/Skoparov/TestRepo.git";
	const char* path = "D:\\Test";

	auto repo = h.openLocalRepo(path);*/

	int i = 0;
}
