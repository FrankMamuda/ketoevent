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
    this->bindVariable( "startTime", this->ui->startTime );
    this->bindVariable( "finishTime", this->ui->finishTime );
    this->bindVariable( "finalTime", this->ui->finalTime );
    this->bindVariable( "penalty", this->ui->penalty );
    this->bindVariable( "singleCombo", this->ui->sCombo );
    this->bindVariable( "doubleCombo", this->ui->dCombo );
    this->bindVariable( "tripleCombo", this->ui->tCombo );
    this->bindVariable( "minMembers", this->ui->min );
    this->bindVariable( "maxMembers", this->ui->max );
    this->bindVariable( "name", this->ui->titleEdit );

    // unlock vars
    this->lockVariables( false );
}
