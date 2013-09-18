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
#ifdef Q_OS_MAC
    // fix ugly spinbox on mac, win32
    this->grid->setMargin( 0 );
#elif defined( Q_OS_WIN )
    this->grid->setMargin( 2 );
#endif

    // set appropriate font
    if ( this->task()->type() == TaskEntry::Special ) {
        font.setItalic( true );
        this->taskName->setFont( font );
    }

    if ( this->task()->isChallenge()) {
        font.setBold( true );
        this->taskName->setFont( font );
    }

    // determine type
    if ( this->task()->type() == TaskEntry::Check ) {
        this->check = new QCheckBox();
        this->check->setMaximumWidth( 48 );
        this->connect( this->check, SIGNAL( clicked()), this, SLOT( saveLog()));
        this->grid->addWidget( this->check, 0, 3, 1, 1 );
	
        // set tooltips
        this->taskName->setToolTip( this->tr( "%1 points, click checkbox to enable/disable" ).arg( this->task()->points()));
        this->check->setToolTip( this->tr( "Toggle task completion" ));
    } else if ( this->task()->type() == TaskEntry::Multi || this->task()->type() == TaskEntry::Special ) {
        this->multi = new QSpinBox();

        if ( this->task()->type() == TaskEntry::Multi )
            this->multi->setMaximum( this->task()->multi());
        else if ( this->task()->type() == TaskEntry::Special )
            this->multi->setMaximum( 10 );

        this->multi->setMaximumWidth( 48 );
        this->connect( this->multi, SIGNAL( editingFinished()), this, SLOT( saveLog()));
        this->grid->addWidget( this->multi, 0, 3, 1, 1 );
	
        // set tooltips
        this->taskName->setToolTip( this->tr( "%1 points (max %2), multiplied by value" ).arg( this->task()->points()).arg( this->task()->multi()));
        this->multi->setToolTip( this->tr( "Change task multiplier" ));
    } else {
        m.error( StrSoftError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set up combo button
    if ( this->task()->type() != TaskEntry::Special ) {
        this->combo = new QPushButton();
        this->combo->setMaximumWidth( 32 );
        this->setComboState( LogEntry::NoCombo );
        this->grid->addWidget( this->combo, 0, 4, 1, 1 );
	
	// set tooltips
	this->combo->setToolTip( this->tr( "Click here to toggle combo state" ));
    }

    // add layout to widget
    this->setLayout( grid );

    // connect combo button for updates
    if ( this->task()->type() != TaskEntry::Special ) {
        this->connect( this->combo, SIGNAL( clicked()), this, SLOT( toggleCombo()));
        this->connect( this->combo, SIGNAL( clicked()), this, SLOT( saveLog()));
    }
}

/*
================
saveLog
================
*/
void TaskWidget::saveLog() {
    int value = 0;

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
    } else if ( this->task()->type() == TaskEntry::Multi || this->task()->type() == TaskEntry::Special ) {
        value = this->multi->value();
    } else {
        m.error( StrFatalError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set to zero - orphans will be deleted anyway
    if ( value <= 0 && this->hasLog()) {
        this->log()->setValue( 0 );
        this->log()->setCombo( LogEntry::NoCombo );
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

    if ( this->task()->type() != TaskEntry::Special )
        this->log()->setCombo( this->comboState());
}

/*
================
setComboState
================
*/
void TaskWidget::setComboState( LogEntry::Combos combo ) {
    if  ( this->task()->type() == TaskEntry::Special ) {
        this->m_comboState = LogEntry::NoCombo;
        return;
    } else
        this->m_comboState = combo;

    // failsafe
    if ( !this->hasTask()) {
        m.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // set text according to combo points
    switch ( this->comboState()) {
    case LogEntry::Single:
        this->combo->setText( QString( "+%1" ).arg( m.var( "combo/single" )->integer()));
        break;

    case LogEntry::Double:
        this->combo->setText( QString( "+%1" ).arg( m.var( "combo/double" )->integer()));
        break;

    case LogEntry::Triple:
        this->combo->setText( QString( "+%1" ).arg( m.var( "combo/triple" )->integer()));
        break;

    case LogEntry::NoCombo:
        this->combo->setText( "+0" );
        break;

    default:
        m.error( StrSoftError + this->tr( "invalid combo state \"%1\"\n" ).arg( static_cast<int>( combo )));
        return;
    }
}

/*
================
toggleCombo
================
*/
void TaskWidget::toggleCombo() {
    if ( this->task()->type() == TaskEntry::Special )
        return;

    // failsafe
    if ( !this->hasTask()) {
        m.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // cycle through combo points
    switch ( this->comboState()) {
    case LogEntry::Single:
        this->setComboState( LogEntry::Double );
        break;

    case LogEntry::Double:
        this->setComboState( LogEntry::Triple );
        break;

    case LogEntry::Triple:
        this->setComboState( LogEntry::NoCombo );
        break;

    case LogEntry::NoCombo:
        this->setComboState( LogEntry::Single );
        break;

    default:
        m.error( StrSoftError + this->tr( "invalid combo state \"%1\"\n" ).arg( static_cast<int>( this->comboState())));
        return;
    }
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

    // clean up
    delete this->taskName;

    if ( this->task()->type() != TaskEntry::Special )
        delete this->combo;

    if ( this->task()->type() == TaskEntry::Check )
        delete this->check;
    else if ( this->task()->type() == TaskEntry::Multi || this->task()->type() == TaskEntry::Special )
        delete this->multi;

    //delete this->points;
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
    else if ( this->task()->type() == TaskEntry::Multi || this->task()->type() == TaskEntry::Special )
        this->multi->setValue( this->log()->value());

    // set combo
    if ( this->task()->type() != TaskEntry::Special )
        this->setComboState( this->log()->combo());
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
    if ( this->task()->type() == TaskEntry::Multi || this->task()->type() == TaskEntry::Special )
        this->multi->setValue( 0 );

    // reset combo
    this->setComboState( LogEntry::NoCombo );
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
}

/*
================
resetTeam
================
*/
void TaskWidget::resetTeam() {
    this->m_team = NULL;
    this->resetLog();
}
