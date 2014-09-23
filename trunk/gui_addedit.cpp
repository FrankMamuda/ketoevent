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
        this->setWindowTitle( this->tr( "Add an event" ));
        this->ui->acceptButton->setText( this->tr( "Add team" ));
        break;

    case ReviewerDialog:
        if ( this->mode() == Edit ) {
            if ( this->id() == -1 ) {
                m.error( StrSoftError, this->tr( "Invalid reviewer id in edit mode\n" ));
                this->reject();
            } else {
                ReviewerEntry *reviewerPtr = m.reviewerForId( this->id());
                if ( reviewerPtr == NULL )
                    return;

                this->ui->titleEdit->setText( reviewerPtr->name());
            }

            this->ui->titleLabel->setText( this->tr( "New name" ));
            this->setWindowTitle( this->tr( "Edit reviewer name" ));
        } else if ( this->mode() == Add ) {
            this->ui->titleLabel->setText( this->tr( "Reviewer name" ));
            this->setWindowTitle( this->tr( "Add a reviewer" ));
            this->ui->acceptButton->setText( this->tr( "Add reviewer" ));
        } else {
            m.error( StrSoftError, this->tr( "Dialog called without an edit mode\n" ));
            return;
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

        // just add a new event
        m.addEvent( this->ui->titleEdit->text());

        // alert parent
        evPtr = qobject_cast<Gui_Event*>( this->parent());
        if ( evPtr != NULL )
            evPtr->fillEvents();
    }
        break;

    case ReviewerDialog:
    {
        if ( this->mode() == Edit ) {
            if ( this->id() == -1 ) {
                m.error( StrSoftError, this->tr( "Invalid reviewer id in edit mode\n" ));
                this->reject();
            } else {
                ReviewerEntry *reviewerPtr = m.reviewerForId( this->id());
                if ( reviewerPtr != NULL )
                    reviewerPtr->setName( this->ui->titleEdit->text());
            }
        } else if ( this->mode() == Add ) {
            // no name - abort
            if ( this->ui->titleEdit->text().isEmpty()) {
                QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
                return;
            }

            // just add a reviewer
            m.addReviewer( this->ui->titleEdit->text());
        } else {
            m.error( StrSoftError, this->tr( "Dialog called without an edit mode\n" ));
            return;
        }
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