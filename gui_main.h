/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
#include "sys_shared.h"
#include "sys_common.h"
#include "gui_teamedit.h"
#include "gui_taskedit.h"

//
// namespaces
//
namespace Ui {
    class Gui_Main;
}

//
// class:Gui_Main
//
class Gui_Main : public QMainWindow {
    Q_OBJECT

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();

public slots:
    void save();
    bool needsSaving();
    void saveReminder();

private slots:
    void on_actionTeams_triggered();
    void fillTeamList();
    void fillTaskList();
    void on_actionSave_triggered();
    void updateView();
    void enableSave();
    void on_teamCombo_activated( int );
    void on_actionAbout_triggered();
    void on_actionTasks_triggered();
    void on_actionRankings_triggered();
    void on_clearButton_clicked();
    void on_findTaskButton_returnPressed();
    void on_findTaskButton_textChanged( const QString & );
    void on_finishTime_timeChanged(const QTime &date);
    void on_actionExit_triggered();
    void on_actionSettings_triggered();
    void toggleSaveFromCvar( QString, QString value );
    void updateDrunkMode( QString, QString value );

private:
    Ui::Gui_Main *ui;
    int lastIndex;
    int currentMatch;

signals:
    void dataChanged();

protected:
    virtual void closeEvent( QCloseEvent * );
};

#endif // GUI_MAIN_H
