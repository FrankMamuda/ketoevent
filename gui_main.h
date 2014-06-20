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
    Q_PROPERTY( int currentReviewerIndex READ currentReviewerIndex WRITE setCurrentReviewerIndex )
    Q_PROPERTY( int currentMatch READ currentMatch WRITE setCurrentMatch )
    Q_PROPERTY( bool locked READ isLocked WRITE setLocked RESET unlock )
    Q_CLASSINFO( "description", "Applet main window" )

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
    int currentTeamIndex() const { return this->m_currentTeamIndex; }
    int currentComboIndex() const { return this->m_currentComboIndex; }
    int currentReviewerIndex() const { return this->m_currentReviewerIndex; }
    int currentMatch() const { return this->m_currentMatch; }
    bool isLocked() const { return m_lock; }

public slots:
    void initialize( bool reload = false );
    void taskIndexChanged( int row );
    void setEventTitle( const QString &name );
    void clearTasks();
    void print( const QString &msg ) { m.print( StrMsg + msg, Main::GuiMain ); }

private slots:
    // team/task/reviewer fill
    void fillTeams( int focedId = -1 );
    void fillTasks();
    void fillReviewers();

    // misc
    void teamIndexChanged( int index );
    void updateFinishTime( QTime time );
    void setCurrentComboIndex( int index = -1 ) { this->m_currentComboIndex = index; }
    void setCurrentTeamIndex( int index = -1 ) { this->m_currentTeamIndex = index; }
    void setCurrentReviewerIndex( int index = -1 ) { this->m_currentReviewerIndex = index; }
    void setLocked( bool locked = true ) { this->m_lock = locked; }
    void unlock() { this->m_lock = false; }

    // search
    void setCurrentMatch( int match = 0 ) { this->m_currentMatch = match; }

    // ui elements
    void on_actionTeams_triggered() { this->setLocked(); Gui_TeamEdit teamEdit( this ); teamEdit.exec(); this->fillTeams(); this->unlock(); }
    void on_actionTasks_triggered() { this->setLocked(); Gui_TaskEdit taskEdit( this ); taskEdit.exec(); this->fillTasks(); this->unlock(); }
    void on_actionRankings_triggered() { this->setLocked(); Gui_Rankings rankings( this ); rankings.exec(); this->unlock(); }
    void on_actionAbout_triggered() { this->setLocked(); Gui_About about( this ); about.exec(); this->unlock(); }
    void on_actionSettings_triggered() { this->setLocked(); Gui_Settings settings( this ); settings.exec(); this->fillTeams(); this->fillTasks(); this->unlock(); }
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
    void on_combineButton_toggled( bool checked );
    void on_actionReviewers_triggered();
    void on_comboReviewers_currentIndexChanged(int index);

protected:
    virtual void closeEvent( QCloseEvent *eventPtr ) { m.shutdown(); QWidget::closeEvent( eventPtr ); }

private:
    Ui::Gui_Main *ui;
    int m_currentTeamIndex;
    int m_currentComboIndex;
    int m_currentMatch;
    int m_currentReviewerIndex;
    bool m_lock;
};

#endif // GUI_MAIN_H
