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
    Q_PROPERTY( int currentTeamIndex READ currentTeamIndex )

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();

public slots:
    void initialize();
    int currentTeamIndex() const { return this->m_currentTeamIndex; }

private slots:
    // team/task fill
    void fillTeams( int focedId = -1 );
    void fillTasks();

    // misc
    void teamIndexChanged( int index );
    void updateFinishTime( QTime time );

    // ui elements
    void on_actionTeams_triggered() { Gui_TeamEdit teamEdit( this ); teamEdit.exec(); this->fillTeams(); }
    void on_actionTasks_triggered() { Gui_TaskEdit taskEdit( this ); taskEdit.exec(); this->fillTasks(); }
    void on_actionRankings_triggered() { Gui_Rankings rankings( this ); rankings.exec(); }
    void on_actionAbout_triggered() { Gui_About about( this ); about.exec(); }
    void on_actionSettings_triggered() { Gui_Settings settings( this ); settings.exec(); this->fillTeams(); this->fillTasks(); }
    void on_actionExit_triggered() { m.shutdown(); }
    void on_logButton_clicked();
    void on_quickAddButton_clicked();
    void on_clearButton_clicked();
    void on_findTaskEdit_textChanged( const QString & );
    void on_findTaskEdit_returnPressed();
    void on_lockButton_clicked();
    void on_actionEvents_triggered();
    void on_actionCombos_triggered();
#ifdef Q_OS_ANDROID
    void on_upButton_clicked();
    void on_downButton_clicked();
#endif

protected:
    virtual void closeEvent( QCloseEvent *eventPtr ) { m.shutdown(); QWidget::closeEvent( eventPtr ); }

private:
    Ui::Gui_Main *ui;
    int m_currentTeamIndex;
    int currentMatch;
};

#endif // GUI_MAIN_H