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
#include "gui_addevent.h"
#include "ui_gui_addevent.h"
#include "gui_event.h"
#include "main.h"
#include <QMessageBox>

/*
================
construct
================
*/
Gui_AddEvent::Gui_AddEvent( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_AddEvent ) {
    ui->setupUi( this );
}

/*
================
destruct
================
*/
Gui_AddEvent::~Gui_AddEvent() {
    delete ui;
}

/*
================
addButton
================
*/
void Gui_AddEvent::on_addButton_clicked() {
    Gui_Event *evPtr;

    // no name - abort
    if ( this->ui->titleEdit->text().isEmpty()) {
        QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
        return;
    }

    // just add a new event
    m.addEvent( this->ui->titleEdit->text());

    // alert parent
    evPtr = qobject_cast<Gui_Event*>( this->parent());
    if ( evPtr != NULL )
        evPtr->fillEvents();

    // we're done
    this->accept();
}
