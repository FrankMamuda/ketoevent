/*
 * Copyright (C) 2013-2018 Factory #12
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

//
// includes
//
#include "addedit.h"
#include "ui_addedit.h"
#include "eventdialog.h"
#include <QMessageBox>

/**
 * @brief AddEdit::AddEdit
 * @param parent
 * @param mode
 * @param id
 * @param parentPtr
 */
AddEdit::AddEdit( Parent parent, Mode mode, int id, QWidget *parentPtr ) : QDialog( parentPtr ), ui( new Ui::AddEdit ) {
    this->ui->setupUi( this );

    // store values
    this->m_id = id;
    this->m_mode = mode;
    this->m_dialogParent = parent;

    switch ( this->dialogParent()) {
    case EventDlg:
        this->ui->titleLabel->setText( this->tr( "Event title" ));
        if ( this->mode() == Rename ) {
            this->setWindowTitle( this->tr( "Rename event" ));
            this->ui->acceptButton->setText( this->tr( "Done" ));
            this->ui->titleEdit->setText( Event::active()->name());
        } else if ( this->mode() == Add ) {
            this->setWindowTitle( this->tr( "Add an event" ));
            this->ui->acceptButton->setText( this->tr( "Add" ));
        }
        break;

    case NoParent:
    default:
        Common::error( StrSoftError, this->tr( "dialog called without parent\n" ));
    }
}

/**
 * @brief AddEdit::~AddEdit
 */
AddEdit::~AddEdit() {
    delete ui;
}

/**
 * @brief AddEdit::on_acceptButton_clicked
 */
void AddEdit::on_acceptButton_clicked() {
    switch ( this->dialogParent()) {
    case EventDlg:
    {
        EventDialog *evPtr;

        // must not be in edit mode
        if ( this->mode() == Edit ) {
            Common::error( StrSoftError, this->tr( "edit mode cannot be used for event dialog\n" ));
            return;
        }

        // no name - abort
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
            return;
        }

        if ( this->mode() == Rename )
            Event::active()->setName( this->ui->titleEdit->text());
        else if ( this->mode() == Add )
            // just add a new event
            Event::add( this->ui->titleEdit->text());

        // alert parent
        evPtr = qobject_cast<EventDialog*>( this->parent());
        if ( evPtr != nullptr )
            evPtr->fillEvents();
    }
        break;

    case NoMode:
    default:
        Common::error( StrSoftError, this->tr( "dialog called without an edit mode\n" ));
        this->reject();
    }

    // we're done
    this->accept();
}

/**
 * @brief AddEdit::on_clearButton_clicked
 */
void AddEdit::on_clearButton_clicked() {
    this->ui->titleEdit->clear();
}
