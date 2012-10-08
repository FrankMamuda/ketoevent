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

//
// includes
//
#include "gui_main.h"
#include "app_main.h"
#include "sys_cvar.h"
#include "ui_gui_main.h"
#include <QMessageBox>
#include "gui_tasklog.h"
#include "gui_rankings.h"
#include "gui_settings.h"
#include "gui_about.h"

//
// cvars
//
extern pCvar *k_eventStartTime;
extern pCvar *k_autoSave;
extern pCvar *k_drunkMode;

/*
================
construct
================
*/
Gui_Main::Gui_Main( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::Gui_Main ) {
    ui->setupUi( this );

    // fill in team list
    this->connect( &m, SIGNAL( teamListChanged()), this, SLOT( fillTeamList()));
    this->connect( &m, SIGNAL( taskListChanged()), this, SLOT( fillTaskList()));
    this->fillTeamList();
    this->lastIndex = this->ui->teamCombo->currentIndex();
    this->ui->finishTime->setMinimumTime( k_eventStartTime->time());
    this->ui->actionSave->setDisabled( true );
    this->connect( this, SIGNAL( dataChanged()), this, SLOT( enableSave()));
    this->connect( k_autoSave, SIGNAL( valueChanged( QString, QString )), this, SLOT( toggleSaveFromCvar( QString,QString )));
    this->fillTaskList();
    this->updateView();
    this->toggleSaveFromCvar( k_autoSave->name(), k_autoSave->string());

    // connect for drunk updates
    this->connect( k_drunkMode, SIGNAL( valueChanged( QString, QString )), this, SLOT( updateDrunkMode( QString, QString )));
    this->updateDrunkMode( k_drunkMode->name(), k_drunkMode->string());
}

/*
================
toggleSaveFromCvar
================
*/
void Gui_Main::toggleSaveFromCvar( QString, QString value ) {
    if ( static_cast<bool>( value.toInt()))
        this->ui->mainToolBar->removeAction( this->ui->actionSave );
    else {
        if ( !this->ui->mainToolBar->actions().contains( this->ui->actionSave ))
            this->ui->mainToolBar->insertAction( this->ui->actionTeams, this->ui->actionSave );
    }
}

/*
================
updateDrunkMode
================
*/
void Gui_Main::updateDrunkMode( QString, QString value ) {
    if ( static_cast<bool>( value.toInt())) {
        this->ui->actionTeams->setDisabled( true );
        this->ui->actionTasks->setDisabled( true );
    } else {
        this->ui->actionTeams->setEnabled( true );
        this->ui->actionTasks->setEnabled( true );
    }
}

/*
================
fillTeamList
================
*/
void Gui_Main::fillTeamList() {
    this->ui->teamCombo->clear();

    if ( m.teamList.isEmpty()) {
        this->ui->finishTime->setDisabled( true );
        this->lastIndex = 0;
        return;
    } else {
        this->ui->finishTime->setEnabled( true );
    }

    foreach ( App_TeamEntry *teamPtr, m.teamList )
        this->ui->teamCombo->addItem( teamPtr->name());
    this->lastIndex = this->ui->teamCombo->currentIndex();
    this->updateView();
}

/*
================
fillTaskList
================
*/
void Gui_Main::fillTaskList() {
    int y;
    QListWidget *lw = this->ui->taskList;

    for ( y = 0; y < lw->count(); y++ ) {
        Gui_TaskLog *taskPtr = qobject_cast<Gui_TaskLog *>( lw->itemWidget( lw->item( y )));
        this->disconnect( taskPtr, SIGNAL( dataChanged()), this, SLOT( enableSave()));
        delete taskPtr;
        delete lw->item( y );
    }
    lw->clear();
    foreach ( App_TaskEntry *entryPtr, m.taskList ) {
        QListWidgetItem *itemPtr = new QListWidgetItem();
        itemPtr->setSizeHint( QSize( 0 , 34 ));
        lw->addItem( itemPtr );
        Gui_TaskLog *taskPtr = new Gui_TaskLog( entryPtr );
        lw->setItemWidget( itemPtr, taskPtr );
        this->connect( taskPtr, SIGNAL( dataChanged()), this, SLOT( enableSave()));
    }
}

/*
================
destruct
================
*/
Gui_Main::~Gui_Main() {
    delete ui;
}

/*
================
actionTeams->triggered
================
*/
void Gui_Main::on_actionTeams_triggered() {
    Gui_TeamEdit teamDialog;

    // save changes if needed
    if ( this->needsSaving())
        this->saveReminder();

    // display team dialog
    teamDialog.exec();

    // update view
    this->updateView();
}

/*
================
closeEvent
================
*/
void Gui_Main::closeEvent( QCloseEvent * ) {
    if ( this->needsSaving())
        this->saveReminder();
    m.shutdown();
}

/*
================
actionSave->triggered
================
*/
void Gui_Main::on_actionSave_triggered() {
    this->save();
    this->ui->actionSave->setDisabled( true );
}

/*
================
enableSave
================
*/
void Gui_Main::enableSave() {
    this->ui->actionSave->setEnabled( true );
}

/*
================
updateView
================
*/
void Gui_Main::updateView() {
    App_TeamEntry *teamPtr = NULL;
    int y;
    QListWidget *lw = this->ui->taskList;

    // disable if team list is empty
    if ( m.teamList.isEmpty())
        this->ui->taskList->setEnabled( false );
    else
        this->ui->taskList->setEnabled( true );

    // get new team log
    teamPtr = m.findTeamByName( this->ui->teamCombo->itemText( this->lastIndex ));
    if ( teamPtr == NULL )
        return;

    // clean up values
    for ( y = 0; y < lw->count(); y++ ) {
        Gui_TaskLog *taskLogPtr = qobject_cast<Gui_TaskLog *>( lw->itemWidget( lw->item( y )));
        taskLogPtr->resetTeam();
    }

    // display new values
    this->ui->finishTime->setTime( teamPtr->finishTime());
    this->ui->actionSave->setDisabled( true );
    for ( y = 0; y < lw->count(); y++ ) {
        Gui_TaskLog *taskLogPtr = qobject_cast<Gui_TaskLog *>( lw->itemWidget( lw->item( y )));
        taskLogPtr->setTeam( teamPtr );
    }
}

/*
================
needsSaving
================
*/
bool Gui_Main::needsSaving() {
    return this->ui->actionSave->isEnabled();
}

/*
================
save
================
*/
void Gui_Main::save() {
    int y;
    App_TeamEntry *teamPtr = NULL;
    QListWidget *lw;

    lw = this->ui->taskList;
    for ( y = 0; y < lw->count(); y++ ) {
        Gui_TaskLog *taskLogPtr = qobject_cast<Gui_TaskLog *>( lw->itemWidget( lw->item( y )));
        if ( taskLogPtr->changed())
            taskLogPtr->saveLog();
    }

    // find team, write out to file
    teamPtr = m.findTeamByName( this->ui->teamCombo->itemText( this->lastIndex ));
    if ( teamPtr != NULL ) {
        teamPtr->setFinishTime( this->ui->finishTime->time());
        teamPtr->save();
    }
}

/*
================
saveReminder
================
*/
void Gui_Main::saveReminder() {
    QMessageBox msgBox;
    int state;

    // failsafe
    if ( this->lastIndex >= m.teamList.count() || this->lastIndex < 0 )
        return;

    // autosave is enabled
    if ( k_autoSave->integer()) {
        this->save();
        return;
    }

    // set up
    msgBox.setText( this->tr( "Team \"%1\" log has been modified" ).arg( this->ui->teamCombo->itemText( this->lastIndex )));
    msgBox.setInformativeText( this->tr( "Do you want to save your changes?" ));
    msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard );
    msgBox.setDefaultButton( QMessageBox::Save );
    msgBox.setWindowIcon( QIcon( ":/icons/keto_48" ));
    msgBox.setIcon( QMessageBox::Question );

    // return restult
    state = msgBox.exec();
    switch ( state ) {
    case QMessageBox::Save:
        this->save();
        break;

    case QMessageBox::Discard:
    default:
        break;
    }
}

/*
================
teamCombo->activated
================
*/
void Gui_Main::on_teamCombo_activated( int index ) {
    if ( index != this->lastIndex && this->needsSaving()) {
        // reset back
        this->ui->teamCombo->setCurrentIndex( this->lastIndex );

        // save changes
        this->saveReminder();
    }
    // reset view
    this->lastIndex = index;
    this->ui->teamCombo->setCurrentIndex( index );
    this->ui->actionSave->setDisabled( true );
    this->updateView();
}

/*
================
actionAbout->triggered
================
*/
void Gui_Main::on_actionAbout_triggered() {
    Gui_About aboutDialog;
    aboutDialog.exec();
}

/*
================
actionTeams->triggered
================
*/
void Gui_Main::on_actionTasks_triggered() {
    Gui_TaskEdit taskDialog;

    // save changes if needed
    if ( this->needsSaving())
        this->saveReminder();

    // display team dialog
    taskDialog.exec();

    // update view
    this->updateView();
}

/*
================
actionRankings->triggered
================
*/
void Gui_Main::on_actionRankings_triggered() {
    Gui_Rankings rankingsDialog( this );

    if ( this->needsSaving())
        this->saveReminder();

    rankingsDialog.exec();
}

/*
================
clearButton->clicked
================
*/
void Gui_Main::on_clearButton_clicked() {
    this->ui->findTaskButton->clear();
}

/*
================
findTaskButton->clicked
================
*/
void Gui_Main::on_findTaskButton_returnPressed() {
    int y;
    QString matchString;
    QModelIndex index;
    bool match = false;

    matchString = this->ui->findTaskButton->text();

    // failsafe
    if ( matchString.isEmpty())
        return;

    // advance
    if ( this->currentMatch >= this->ui->taskList->count() - 1 || this->currentMatch <= 0 )
        this->currentMatch = 0;
    else
        this->currentMatch++;

    // find item from current position
    for ( y = this->currentMatch; y < this->ui->taskList->count(); y++ ) {
        if ( m.taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
            match = true;
            currentMatch = y;
            break;
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->ui->taskList->count(); y++ ) {
            if ( m.taskList.at( y )->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                currentMatch = y;
                break;
            }
        }
    }

    // matched?
    if ( match ) {
        this->ui->taskList->setCurrentRow( y );
    } else {
        this->ui->taskList->clearSelection();
        QPalette p( this->ui->findTaskButton->palette());
        p.setColor( QPalette::Base, QColor( 255, 0, 0, 64 ));
        this->ui->findTaskButton->setPalette( p );
    }
}

/*
================
findTaskButton->textChanged
================
*/
void Gui_Main::on_findTaskButton_textChanged( const QString & ) {
    if ( this->ui->findTaskButton->palette().color( QPalette::Base ) == QColor( 255, 0, 0, 64 )) {
        QPalette p( this->ui->findTaskButton->palette());
        p.setColor( QPalette::Base, Qt::white );
        this->ui->findTaskButton->setPalette( p );
    }
}

/*
================
finishTime->timeChanged
================
*/
void Gui_Main::on_finishTime_timeChanged( const QTime & ) {
    emit this->dataChanged();
}

/*
================
actionExit->triggered
================
*/
void Gui_Main::on_actionExit_triggered() {
    if ( this->needsSaving())
        this->saveReminder();
    m.shutdown();
}

/*
================
actionSettings->triggered
================
*/
void Gui_Main::on_actionSettings_triggered() {
    Gui_Settings settingsDialog;

    if ( this->needsSaving())
        this->saveReminder();

    settingsDialog.exec();
}
