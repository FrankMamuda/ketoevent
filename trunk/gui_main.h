/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

//
// includes
//
#include <QMainWindow>
#include <QCloseEvent>
#include <QTime>
#include "gui_teamedit.h"
#include "gui_taskedit.h"
#include "gui_rankings.h"
#include "gui_about.h"
#include "gui_settings.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Main;
}

//
// class: Gui_Main
//
class Gui_Main : public QMainWindow {
    Q_OBJECT
    
public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
    
private:
    Ui::Gui_Main *ui;

public slots:
    void updateView();
    void initialize();
    void teamIndexChanged( int index );
    void updateFinishTime( QTime time );

protected:
    virtual void closeEvent( QCloseEvent * );

private slots:
    void on_actionTeams_triggered() { Gui_TeamEdit teamEdit( this ); teamEdit.exec(); this->updateView(); }
    void on_actionTasks_triggered() { Gui_TaskEdit taskEdit( this ); taskEdit.exec(); this->updateView(); }
    void on_actionRankings_triggered() { Gui_Rankings rankings( this ); rankings.exec(); }
    void on_actionAbout_triggered() { Gui_About about( this ); about.exec(); }
    void on_actionSettings_triggered() { Gui_Settings settings( this ); settings.exec(); }
    void on_actionExit_triggered();
    void fillTaskList();
};

#endif // GUI_MAIN_H
