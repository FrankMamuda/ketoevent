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
#include "gui_event.h"
#include "ui_gui_event.h"

/*
================
construct
================
*/
Gui_Event::Gui_Event( QWidget *parent ) : Gui_SettingsDialog( parent ), ui( new Ui::Gui_Event ) {
    ui->setupUi( this );

    if ( m.isInitialized())
        this->bindVars();
}

/*
================
destruct
================
*/
Gui_Event::~Gui_Event() {
    this->unbindVars();
    delete ui;
}

/*
================
bindVars
================
*/
void Gui_Event::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    //this->bindVariable( "key", object );
    //this->bindVariable( "name", this->ui->titleEdit );

    // unlock vars
    this->lockVariables( false );
}

// bind vars
#if 0
m.svar( "startTime" )->bind( this->ui->startTime, qobject_cast<QObject*>( this ));
m.svar( "finishTime" )->bind( this->ui->finishTime, qobject_cast<QObject*>( this ));
m.svar( "finalTime" )->bind( this->ui->finalTime, qobject_cast<QObject*>( this ));
m.svar( "penalty" )->bind( this->ui->penalty, qobject_cast<QObject*>( this ));
m.svar( "singleCombo" )->bind( this->ui->sCombo, qobject_cast<QObject*>( this ));
m.svar( "doubleCombo" )->bind( this->ui->dCombo, qobject_cast<QObject*>( this ));
m.svar( "tripleCombo" )->bind( this->ui->tCombo, qobject_cast<QObject*>( this ));
m.svar( "minMembers" )->bind( this->ui->min, qobject_cast<QObject*>( this ));
m.svar( "maxMembers" )->bind( this->ui->max, qobject_cast<QObject*>( this ));
m.svar( "name" )->bind( this->ui->titleEdit, qobject_cast<QObject*>( this ));
#endif
