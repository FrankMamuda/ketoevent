/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
    Q_PROPERTY( int currentTeamIndex READ currentTeamIndex WRITE setCurrentTeamIndex )
    Q_PROPERTY( int currentComboIndex READ currentComboIndex WRITE setCurrentComboIndex )
    Q_PROPERTY( int currentMatch READ currentMatch WRITE setCurrentMatch )
    Q_CLASSINFO( "description", "Applet main window" )

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
    int currentTeamIndex() const { return this->m_currentTeamIndex; }
    int currentComboIndex() const { return this->m_currentComboIndex; }
    int currentMatch() const { return this->m_currentMatch; }

public slots:
    void initialize( bool reload = false );
    void taskIndexChanged( int row );
    void setEventTitle();
    void clearTasks();
    void print( const QString &msg ) { m.print( StrMsg + msg, Main::GuiMain ); }
    void updateStatusBar();

private slots:
    // team/task fill
    void fillTeams( int focedId = -1 );
    void fillTasks();

    // misc
    void teamIndexChanged( int index );
    void updateFinishTime( QTime time );
    void setCurrentComboIndex( int index = -1 ) { this->m_currentComboIndex = index; }
    void setCurrentTeamIndex( int index = -1 ) { this->m_currentTeamIndex = index; }

    // search
    void setCurrentMatch( int match = 0 ) { this->m_currentMatch = match; }

    // ui elements
    void on_actionTeams_triggered() { Gui_TeamEdit teamEdit( this ); teamEdit.exec(); this->fillTeams(); }
    void on_actionTasks_triggered() { Gui_TaskEdit taskEdit( this ); taskEdit.exec(); this->fillTasks(); }
    void on_actionRankings_triggered() { Gui_Rankings rankings( this ); rankings.exec(); }
    void on_actionAbout_triggered() { Gui_About about( this ); about.exec(); }
    void on_actionSettings_triggered() { Gui_Settings settings( this ); settings.exec(); this->fillTeams(); this->fillTasks(); this->setEventTitle(); }
    void on_actionExit_triggered() { m.shutdown(); }
    void on_logButton_clicked();
    void on_quickAddButton_clicked();
    void on_clearButton_clicked();
    void on_findTaskEdit_textChanged( const QString & );
    void on_findTaskEdit_returnPressed();
    void on_lockButton_clicked();
    void on_actionEvents_triggered();
    void on_actionCombos_triggered();
    void on_combineButton_toggled( bool checked );
    void on_actionConsole_toggled( bool visible );

protected:
    virtual void closeEvent( QCloseEvent *eventPtr ) { m.shutdown(); QWidget::closeEvent( eventPtr ); }

private:
    Ui::Gui_Main *ui;
    int m_currentTeamIndex;
    int m_currentComboIndex;
    int m_currentMatch;
};

#endif // GUI_MAIN_H
