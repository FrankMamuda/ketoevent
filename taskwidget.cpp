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

//
// includes
//
#include "taskwidget.h"
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include "gui_main.h"

//
// FIXME: does not save COMBO anymore!
//

/*
================
construct
================
*/
TaskWidget::TaskWidget( TaskEntry *parentPtr ) {
    QFont font;

    // set task
    this->setTask( parentPtr );

    // failsafe
    if ( !this->hasTask()) {
        m.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // create widget
    this->grid = new QGridLayout();
    this->taskName = new QLabel( this->task()->name());
    this->grid->addWidget( this->taskName, 0, 0, 1, 3 );

    // fix ugly spinbox on mac, win32 (and systems with DPI!=96)
    this->grid->setMargin( 0 );

    // get font
    font = this->taskName->font();

    // set appropriate font
    if ( this->task()->style() == TaskEntry::Italic ) {
        font.setItalic( true );
        this->taskName->setFont( font );
    } else if ( this->task()->style() == TaskEntry::Bold ) {
        font.setBold( true );
    }

    // set font
    this->taskName->setFont( font );

    // determine type
    if ( this->task()->type() == TaskEntry::Check ) {
        this->check = new QCheckBox();
        this->check->setMaximumWidth( 48 );
        this->check->setGeometry( this->check->x(), this->check->y(), this->check->width(), this->check->height() * 2 );
        this->connect( this->check, SIGNAL( clicked()), this, SLOT( saveLog()));
        this->grid->addWidget( this->check, 0, 3, 1, 1 );

        // set tooltips
        this->taskName->setToolTip( this->tr( "%1 points, click checkbox to enable/disable" ).arg( this->task()->points()));
        this->check->setToolTip( this->tr( "Toggle task completion" ));
    } else if ( this->task()->type() == TaskEntry::Multi ) {
        this->multi = new QSpinBox();

        if ( this->task()->type() == TaskEntry::Multi )
            this->multi->setMaximum( this->task()->multi());

#ifndef Q_OS_ANDROID
        this->multi->setMaximumWidth( 48 );
#endif
        this->connect( this->multi, SIGNAL( valueChanged( int )), this, SLOT( saveLog()));
        this->grid->addWidget( this->multi, 0, 3, 1, 1 );

        // set tooltips
        this->taskName->setToolTip( this->tr( "%1 points (max %2), multiplied by value" ).arg( this->task()->points()).arg( this->task()->multi()));
        this->multi->setToolTip( this->tr( "Change task multiplier" ));
    } else {
        m.error( StrSoftError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set up combo button
    this->combo = new QPushButton();
#ifdef Q_OS_ANDROID
    // make it larger for easier hits
    this->combo->setMaximumWidth( 96 );
#else
    this->combo->setMaximumWidth( 32 );
#endif
    //this->setComboState( LogEntry::NoCombo );
    this->grid->addWidget( this->combo, 0, 4, 1, 1 );

    // set tooltips
    this->combo->setToolTip( this->tr( "Click here to toggle combo state" ));

    // disable combo button by default
    this->combo->hide();
    this->combo->setIcon( QIcon( ":/icons/task_add_16" ));
    this->combo->setCheckable( true );

    // add layout to widget
    this->setLayout( grid );

    // set default stylesheet
    this->taskName->setStyleSheet( "padding: 6px;" );

    // connect combo button for updates
    this->connect( this->combo, SIGNAL( toggled( bool )) /*SIGNAL( clicked())*/, this, SLOT( toggleCombo( bool )));
}

/*
================
saveLog
================
*/
void TaskWidget::saveLog() {
    int value = 0;

    if ( !this->m_active ) {
        //m.print( "log inactive, ignoring save\n" );
        return;
    }

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        m.error( StrFatalError + this->tr( "task or team not set\n" ));
        return;
    }

    // determine type
    if ( this->task()->type() == TaskEntry::Check ) {
        if ( this->check->checkState() == Qt::Checked )
            value = true;
        else
            value = false;
    } else if ( this->task()->type() == TaskEntry::Multi ) {
        value = this->multi->value();
    } else {
        m.error( StrFatalError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set to zero - orphans will be deleted anyway
    if ( value <= 0 && this->hasLog()) {
        this->log()->setValue( 0 );
        //this->log()->setCombo( LogEntry::NoCombo );
    }

    // no log?, no problem - create one
    if ( !this->hasLog()) {
        // check for local logs
        // set log
        LogEntry *logPtr = m.logForIds( this->team()->id(), this->task()->id());
        if ( logPtr != NULL ) {
            this->setLog( logPtr, false );
        } else {
            // add new log
            logPtr = m.addLog( this->task()->id(), this->team()->id());
            this->setLog( logPtr, true );
            this->team()->logList << this->log();
        }
    }
    this->log()->setValue( value );
    this->team()->logList.last()->setValue( value );
    //this->log()->setCombo( this->comboState());
}

/*
================
destruct
================
*/
TaskWidget::~TaskWidget() {
    // failsafe
    if ( !this->hasTask()) {
        m.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // disconnect buttons and spinboxes
    //this->disconnect( this->combo, SIGNAL( clicked()));

    if ( this->task()->type() == TaskEntry::Check && this->check != NULL ) {
        this->disconnect( this->check, SIGNAL( clicked()));
        delete this->check;
    } else if ( this->task()->type() == TaskEntry::Multi && this->multi != NULL ) {
        this->disconnect( this->multi, SIGNAL( valueChanged( int )));
        delete this->multi;
    }

    // clean up
    delete this->taskName;
    delete this->combo;
    delete this->grid;
}

/*
================
setLog
================
*/
void TaskWidget::setLog( LogEntry *logPtr, bool fromDatabase ) {
    this->m_log = logPtr;

    // failsafe
    if ( !this->hasTask()) {
        m.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // failsafe
    if ( !this->hasLog()) {
        m.error( StrSoftError + this->tr( "log not set\n" ));
        return;
    }

    // failsafe
    if ( !this->hasTeam()) {
        m.error( StrSoftError + this->tr( "team not set\n" ));
        return;
    }

    TaskEntry *taskPtr = m.taskForId( logPtr->taskId());
    if ( taskPtr == NULL )
        return;

    // make sure log's task is the same task
    if ( taskPtr != this->task())
        return;

    // just initial addition
    if ( fromDatabase )
        return;

    // set values
    if ( this->task()->type() == TaskEntry::Check )
        this->check->setChecked( this->log()->check());
    else if ( this->task()->type() == TaskEntry::Multi )
        this->multi->setValue( this->log()->value());

    // set combo
    //this->setComboState( this->log()->combo());
}

/*
================
resetLog
================
*/
void TaskWidget::resetLog() {
    this->m_log = NULL;

    // failsafe - this really should not happen
    if ( !this->hasTask()) {
        m.error( StrFatalError + this->tr( "task not set\n" ));
        return;
    }

    // reset values
    if ( this->task()->type() == TaskEntry::Check )
        this->check->setChecked( false );
    if ( this->task()->type() == TaskEntry::Multi )
        this->multi->setValue( 0 );

    // reset combo
    //this->setComboState( LogEntry::NoCombo );
}

/*
================
setTeam
================
*/
void TaskWidget::setTeam( TeamEntry *teamPtr ) {
    this->m_team = teamPtr;

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        m.error( StrSoftError + this->tr( "task, team or log not set\n" ));
        return;
    }

    // find appropriate log
    foreach ( LogEntry *logPtr, teamPtr->logList ) {
        TaskEntry *taskPtr = m.taskForId( logPtr->taskId());

        if ( taskPtr == NULL )
            continue;

        if ( taskPtr == this->task()) {
            this->setLog( logPtr );
            break;
        }
    }

    // enable - TODO: props!!!
    this->m_active = true;
}

/*
================
resetTeam
================
*/
void TaskWidget::resetTeam() {
    this->m_team = NULL;
    this->m_active = false;
    this->resetLog();
}

/*
================
toggleCombo
================
*/
void TaskWidget::toggleCombo( bool checked ) {    
    // TODO: change to different icons
    if ( checked ) {
        this->taskName->setStyleSheet( "background-color: rgba( 0, 200, 0, 128 ); border-style: outset; border-width: 2px; border-radius: 10px; border-color: beige; font: bold; padding: 6px;" );
        this->combo->setIcon( QIcon( ":/icons/task_remove_16" ));

        if ( !this->hasLog() && this->hasCombo())
            return;

        Gui_Main *gui = qobject_cast<Gui_Main *>( m.parent );
        if ( gui == NULL )
            return;

        this->log()->setComboId( gui->currentComboIndex());
    } else {
        this->taskName->setStyleSheet( "padding: 6px;" );
        this->combo->setIcon( QIcon( ":/icons/task_add_16" ));


        if ( !this->hasLog() && this->hasCombo())
            return;

        this->log()->setComboId( -1 );

        //if ( this->hasCombo() && this->hasLog())
        //    this->log->setComboId();
    }
}
