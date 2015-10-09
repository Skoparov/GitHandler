#ifndef GITUPDATER_H
#define GITUPDATER_H

#include <QMainWindow>
#include "ui_gitupdater.h"

#include "GitHandler.h"

//////////////////////////////////////////////////////////////////////////////
///////////////				 GitUpdater				    //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitUpdater : public QMainWindow
{
	Q_OBJECT

public:
	GitUpdater(QWidget *parent = 0);
	~GitUpdater();
	
private:
	Ui::GitUpdaterClass ui;
};

#endif // GITUPDATER_H
