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
#include "gui_addedit.h"
#include "ui_gui_addedit.h"
#include "gui_event.h"
#include <QMessageBox>

/*
================
construct
================
*/
Gui_AddEdit::Gui_AddEdit( Parent parent, Mode mode, int id, QWidget *parentPtr ) : QDialog( parentPtr ), ui( new Ui::Gui_AddEdit ) {
    ui->setupUi( this );

    // store values
    this->m_id = id;
    this->m_mode = mode;
    this->m_dialogParent = parent;

    switch ( this->dialogParent()) {
    case EventDialog:
        this->ui->titleLabel->setText( this->tr( "Event title" ));
        if ( this->mode() == Rename ) {
            this->setWindowTitle( this->tr( "Rename event" ));
            this->ui->acceptButton->setText( this->tr( "Done" ));
            this->ui->titleEdit->setText( m.currentEvent()->name());
        } else if ( this->mode() == Add ) {
            this->setWindowTitle( this->tr( "Add an event" ));
            this->ui->acceptButton->setText( this->tr( "Add team" ));
        }
        break;

    case NoParent:
    default:
        m.error( StrSoftError, this->tr( "Dialog called without parent\n" ));
    }
}

/*
================
destruct
================
*/
Gui_AddEdit::~Gui_AddEdit() {
    delete ui;
}

/*
================
acceptButton->clicked
================
*/
void Gui_AddEdit::on_acceptButton_clicked() {
    switch ( this->dialogParent()) {
    case EventDialog:
    {
        Gui_Event *evPtr;

        // must not be in edit mode
        if ( this->mode() == Edit ) {
            m.error( StrSoftError, this->tr( "Edit mode cannot be used for event dialog\n" ));
            return;
        }

        // no name - abort
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
            return;
        }

        if ( this->mode() == Rename )
            m.currentEvent()->setName( this->ui->titleEdit->text());
        else if ( this->mode() == Add )
            // just add a new event
            m.addEvent( this->ui->titleEdit->text());

        // alert parent
        evPtr = qobject_cast<Gui_Event*>( this->parent());
        if ( evPtr != NULL )
            evPtr->fillEvents();
    }
        break;

    case NoMode:
    default:
        m.error( StrSoftError, this->tr( "Dialog called without an edit mode\n" ));
        this->reject();
    }

    // we're done
    this->accept();
}

/*
================
clearButton->clicked
================
*/
void Gui_AddEdit::on_clearButton_clicked() {
    this->ui->titleEdit->clear();
}
