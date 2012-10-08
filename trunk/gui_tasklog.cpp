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
#include "gui_tasklog.h"
#include "sys_common.h"
#include "sys_cvar.h"

// cvars
extern pCvar *k_singleComboPoints;
extern pCvar *k_doubleComboPoints;
extern pCvar *k_tripleComboPoints;

/*
================
construct
================
*/
Gui_TaskLog::Gui_TaskLog( App_TaskEntry *parentPtr ) {
    QFont font;

    // set task
    this->setTask( parentPtr );

    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // create widget
    this->grid = new QGridLayout();
    this->taskName = new QLabel( this->task()->name());
    this->grid->addWidget( this->taskName, 0, 0, 1, 3 );

    // set appropriate font
    if ( this->task()->type() == App_TaskEntry::Special ) {
        font.setItalic( true );
        this->taskName->setFont( font );
    }

    if ( this->task()->isChallenge()) {
        font.setBold( true );
        this->taskName->setFont( font );
    }

    // determine type
    if ( this->task()->type() == App_TaskEntry::Check ) {
        this->check = new QCheckBox();
        this->check->setMaximumWidth( 48 );
        this->connect( this->check, SIGNAL( clicked()), this, SLOT( update()));
        this->grid->addWidget( this->check, 0, 3, 1, 1 );
    } else if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special ) {
        this->multi = new QSpinBox();

        if ( this->task()->type() == App_TaskEntry::Multi )
            this->multi->setMaximum( this->task()->maxMulti());
        else if ( this->task()->type() == App_TaskEntry::Special )
            this->multi->setMaximum( 10 );

        this->multi->setMaximumWidth( 48 );
        this->connect( this->multi, SIGNAL( editingFinished()), this, SLOT( update()));
        this->grid->addWidget( this->multi, 0, 3, 1, 1 );
    } else {
        com.error( StrSoftError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set up combo button
    if ( this->task()->type() != App_TaskEntry::Special ) {
        this->combo = new QPushButton();
        this->combo->setMaximumWidth( 32 );
        this->setComboState( App_TaskLog::NoCombo );
        this->grid->addWidget( this->combo, 0, 4, 1, 1 );
    }

    // add points display
    /*this->points = new QLineEdit();
    this->points->setText( "0" );
    this->points->setMaximumWidth( 32 );
    this->grid->addWidget( this->points, 0, 5, 1, 1 );*/

    // add layout to widget
    this->setLayout( grid );

    // connect combo button for updates
    if ( this->task()->type() != App_TaskEntry::Special ) {
        this->connect( this->combo, SIGNAL( clicked()), this, SLOT( toggleCombo()));
        this->connect( this->combo, SIGNAL( clicked()), this, SLOT( update()));
    }

    // nothing changed
    this->setChanged( false );
}

/*
================
update
================
*/
void Gui_TaskLog::update() {
    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // failsafe
    if ( !this->hasTeam()) {
        com.error( StrSoftError + this->tr( "team not set\n" ));
        return;
    }

    emit this->dataChanged();
    //this->points->setText( QString( "%1" ).arg( this->log()->calculate()));
    this->setChanged();
}

/*
================
saveLog
================
*/
void Gui_TaskLog::saveLog() {
    int value = 0;

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        com.error( StrSoftError + this->tr( "task or team not set\n" ));
        return;
    }

    // ignore zero values
    if ( !this->changed())
        return;

    // determine type
    if ( this->task()->type() == App_TaskEntry::Check ) {
        if ( this->check->checkState() == Qt::Checked )
            value = true;
        else
            value = false;
    } else if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special ) {
        value = this->multi->value();
    } else {
        com.error( StrSoftError + this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // no value, abort
    if ( value <= 0 ) {
        this->team()->logList.removeOne( this->log());
        delete m_log;
        this->m_log = NULL;
        return;
    }

    // no log?, no problem - create one
    if ( !this->hasLog()) {
        this->setLog( new App_TaskLog());
        this->team()->logList << this->log();
    }

    this->log()->setValue( value );
    this->log()->setTask( this->task());
    if ( this->task()->type() != App_TaskEntry::Special )
        this->log()->setCombo( this->comboState());
}

/*
================
setComboState
================
*/
void Gui_TaskLog::setComboState( App_TaskLog::Combos combo ) {
    if  ( this->task()->type() == App_TaskEntry::Special ) {
        this->m_comboState = App_TaskLog::NoCombo;
        return;
    } else
        this->m_comboState = combo;

    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // set text according to combo points
    switch ( this->comboState()) {
    case App_TaskLog::Single:
        this->combo->setText( QString( "+%1" ).arg( k_singleComboPoints->integer()));
        break;

    case App_TaskLog::Double:
        this->combo->setText( QString( "+%1" ).arg( k_doubleComboPoints->integer()));
        break;

    case App_TaskLog::Triple:
        this->combo->setText( QString( "+%1" ).arg( k_tripleComboPoints->integer()));
        break;

    case App_TaskLog::NoCombo:
        this->combo->setText( "+0" );
        break;

    default:
        com.error( StrSoftError + this->tr( "invalid combo state \"%1\"\n" ).arg( static_cast<int>( combo )));
        return;
    }
}

/*
================
toggleCombo
================
*/
void Gui_TaskLog::toggleCombo() {
    if ( this->task()->type() == App_TaskEntry::Special )
        return;

    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // cycle through combo points
    switch ( this->comboState()) {
    case App_TaskLog::Single:
        this->setComboState( App_TaskLog::Double );
        break;

    case App_TaskLog::Double:
        this->setComboState( App_TaskLog::Triple );
        break;

    case App_TaskLog::Triple:
        this->setComboState( App_TaskLog::NoCombo );
        break;

    case App_TaskLog::NoCombo:
        this->setComboState( App_TaskLog::Single );
        break;

    default:
        com.error( StrSoftError + this->tr( "invalid combo state \"%1\"\n" ).arg( static_cast<int>( this->comboState())));
        return;
    }
}

/*
================
destruct
================
*/
Gui_TaskLog::~Gui_TaskLog() {
    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // clean up
    delete this->taskName;

    if ( this->task()->type() != App_TaskEntry::Special )
        delete this->combo;

    if ( this->task()->type() == App_TaskEntry::Check )
        delete this->check;
    else if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special )
        delete this->multi;

    //delete this->points;
    delete this->grid;
}

/*
================
setLog
================
*/
void Gui_TaskLog::setLog( App_TaskLog *logPtr ) {
    this->m_log = logPtr;

    // failsafe
    if ( !this->hasTask() || !this->hasLog() || !this->hasTeam()) {
        com.error( StrSoftError + this->tr( "task, team or log not set\n" ));
        return;
    }

    // make sure log's task is the same task
    if ( this->log()->task() != this->task())
        return;

    // set values
    if ( this->task()->type() == App_TaskEntry::Check )
        this->check->setChecked( this->log()->check());
    else if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special )
        this->multi->setValue( this->log()->value());

    // set combo
    if ( this->task()->type() != App_TaskEntry::Special )
        this->setComboState( this->log()->combo());
}

/*
================
resetLog
================
*/
void Gui_TaskLog::resetLog() {
    this->m_log = NULL;

    // failsafe
    if ( !this->hasTask()) {
        com.error( StrSoftError + this->tr( "task not set\n" ));
        return;
    }

    // reset values
    if ( this->task()->type() == App_TaskEntry::Check )
        this->check->setChecked( false );
    if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special )
        this->multi->setValue( 0 );

    // reset combo
    this->setComboState( App_TaskLog::NoCombo );
}

/*
================
setTeam
================
*/
void Gui_TaskLog::setTeam( App_TeamEntry *teamPtr ) {
    this->m_team = teamPtr;

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        com.error( StrSoftError + this->tr( "task, team or log not set\n" ));
        return;
    }

    // find appropriate log
    foreach ( App_TaskLog *logPtr, teamPtr->logList ) {
        if ( logPtr->task() == this->task()) {
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
void Gui_TaskLog::resetTeam() {
    this->m_team = NULL;
    this->resetLog();
}
