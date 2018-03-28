/*
 * Copyright (C) 2013-2018 Factory #12
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#pragma once

//
// includes
//
#include <QMainWindow>
#include <QCloseEvent>
#include <QTime>
#include "teamdialog.h"
#include "taskdialog.h"
#include "rankings.h"
#include "settings.h"
#include "eventdialog.h"

//
// namespace: Ui
//
namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY( int currentTeamIndex READ currentTeamIndex WRITE setCurrentTeamIndex )
    Q_PROPERTY( int currentComboIndex READ currentComboIndex WRITE setCurrentComboIndex )
    Q_PROPERTY( int currentMatch READ currentMatch WRITE setCurrentMatch )
    Q_PROPERTY( int lastEventId READ lastEventId WRITE setLastEventId )
    Q_CLASSINFO( "description", "Applet main window" )

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();
    int currentTeamIndex() const { return this->m_currentTeamIndex; }
    int currentComboIndex() const { return this->m_currentComboIndex; }
    int currentMatch() const { return this->m_currentMatch; }
    int currentTeamId();
    int lastEventId() const { return this->m_lastEventId; }

public slots:
    void initialise( bool reload = false );
    void taskIndexChanged( int row );
    void setEventTitle();
    void clearTasks();
    void lock();
#ifdef APPLET_DEBUG
    void stressTest( int numTeams = 1 );
    void testTeam( Team *team );
#endif
    // team/task fill
    void selectTeam( int id = -1 );
    void fillTasks();

private slots:

    // misc
    void teamIndexChanged( int index );
    void updateFinishTime( QTime time );
    void setCurrentComboIndex( int index = -1 ) { this->m_currentComboIndex = index; }
    void setCurrentTeamIndex( int index = -1 ) { this->m_currentTeamIndex = index; }
    void setLastEventId( int id ) { this->m_lastEventId = id; }
    void testSortButton();
    void eventDialogClosed( int signal );
    void teamDialogClosed( int signal );
    void taskDialogClosed( int signal );
    void settingsDialogClosed( int signal );

    // search
    void setCurrentMatch( int match = 0 ) { this->m_currentMatch = match; }

    // ui elements
    void on_actionTeams_triggered() { this->teamDialog->show(); this->selectTeam(); }
    void on_actionTasks_triggered() { this->taskDialog->show(); }
    void on_actionRankings_triggered() { this->rankingsDialog->show(); }
    void on_actionSettings_triggered();
    void on_actionExit_triggered() { Main::instance()->shutdown(); }
    void on_clearButton_clicked();
    void on_findTaskEdit_textChanged( const QString & );
    void on_findTaskEdit_returnPressed();
    void on_actionEvents_triggered();
    void on_actionCombos_triggered();
    void on_actionConsole_toggled( bool );
    void on_actionLogTime_triggered();
    void on_actionCombine_toggled( bool );
    void on_actionQuickAdd_triggered();
    void on_actionSort_triggered();
    void on_actionLockTeam_triggered();
    void on_actionCombine_changed();

protected:
    virtual void closeEvent( QCloseEvent *event ) { Main::instance()->shutdown(); QWidget::closeEvent( event ); }

private:
    Ui::MainWindow *ui;
    EventDialog *eventDialog;
    TeamDialog *teamDialog;
    TaskDialog *taskDialog;
    Rankings *rankingsDialog;
    Settings *settingsDialog;
    int m_currentTeamIndex;
    int m_currentComboIndex;
    int m_currentMatch;
    int m_lastEventId;
};
