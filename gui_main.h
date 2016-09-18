/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

//
// includes
//
#include <QMainWindow>
#include <QCloseEvent>
#include <QTime>
#include "gui_team.h"
#include "gui_task.h"
#include "gui_rankings.h"
#include "gui_about.h"
#include "gui_settings.h"
#include "gui_event.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Main;
}

/**
 * @brief The Gui_Main class
 */
class Gui_Main : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY( int currentTeamIndex READ currentTeamIndex WRITE setCurrentTeamIndex )
    Q_PROPERTY( int currentComboIndex READ currentComboIndex WRITE setCurrentComboIndex )
    Q_PROPERTY( int currentMatch READ currentMatch WRITE setCurrentMatch )
    Q_PROPERTY( int lastEventId READ lastEventId WRITE setLastEventId )
    Q_CLASSINFO( "description", "Applet main window" )

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
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
    void print( const QString &msg ) { m.print( StrMsg + msg, Main::GuiMain ); }
    void lock();
#ifdef APPLET_DEBUG
    void stressTest( int numTeams = 1 );
    void testTeam( Team *teamPtr );
#endif
    // team/task fill
    void fillTeams( int focedId = -1 );
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
    void on_actionTeams_triggered() { this->teamDialog->show(); this->fillTeams(); }
    void on_actionTasks_triggered() { this->taskDialog->show(); }
    void on_actionRankings_triggered() { this->rankingsDialog->show(); }
    void on_actionAbout_triggered() { Gui_About about( this ); about.exec(); }
    void on_actionSettings_triggered();
    void on_actionExit_triggered() { m.shutdown(); }
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

protected:
    virtual void closeEvent( QCloseEvent *eventPtr ) { m.shutdown(); QWidget::closeEvent( eventPtr ); }

private:
    Ui::Gui_Main *ui;
    Gui_Event *eventDialog;
    Gui_Team *teamDialog;
    Gui_Task *taskDialog;
    Gui_Rankings *rankingsDialog;
    Gui_Settings *settingsDialog;
    int m_currentTeamIndex;
    int m_currentComboIndex;
    int m_currentMatch;
    int m_lastEventId;
};

#endif // GUI_MAIN_H
