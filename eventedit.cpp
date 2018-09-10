/*
 * Copyright (C) 2018 Factory #12
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
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "eventedit.h"
#include "ui_eventedit.h"
#include "variable.h"
#include <QMessageBox>

/**
 * @brief EventEdit::EventEdit
 * @param parent
 */
EventEdit::EventEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::EventEdit ), m_edit( false ) {
    const Row event = Event::instance()->row( MainWindow::instance()->currentEventId());

    // set up ui
    this->ui->setupUi( this );

    if ( event == Row::Invalid )
        return;

    // only visible in quick add
    this->setWindowTitle( this->tr( "Add event" ));
    this->setWindowIcon( QIcon( ":/icons/ketone" ));

    // empty event title check
    auto emptyTitle = [ this ]() {
        // warn upon empty event title
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::information( this, this->tr( "Empty event title" ), this->tr( "Please enter event title" ));
            return true;
        }
        return false;
    };

    // add button action
    this->connect( this->ui->addButton, &QPushButton::clicked, [ this, emptyTitle ]() {
        const QString eventTitle( this->ui->titleEdit->text());

        // abort on empty event title
        if ( emptyTitle())
            return;

        // abort on existing event
        if ( Event::instance()->contains( Event::Title, eventTitle ) && !this->isEditing()) {
            QMessageBox::information( this, this->tr( "Event already exists" ), this->tr( "Event already exists\nChoose a different title" ));
            return;
        }

        // if everything is ok, add a new event
        /*Id eventId = Id::Invalid;*/
        if ( !this->isEditing()) {
            Event::instance()->add( eventTitle );
            // TODO: set values

        } else {
            const Row event = Event::instance()->indexToRow( EditorDialog::instance()->container->currentIndex().row());

            if ( event == Row::Invalid )
                return;

            Event::instance()->setTitle( event, eventTitle );
        }

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else {
            // TODO:
            //if ( eventId != Id::Invalid )
            //    MainWindow::instance()->setCurrentEvent( eventId );

            this->close();
        }
    });

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, [ this ]() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief EventEdit::~EventEdit
 */
EventEdit::~EventEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));
    this->disconnect( this->ui->titleEdit, SIGNAL( returnPressed()));

    // delete ui
    delete this->ui;
}

/**
 * @brief EventEdit::reset
 * @param edit
 */
void EventEdit::reset( bool edit ) {
    this->m_edit = edit;

    if ( !this->isEditing()) {
        // reset ui components to default values
        this->ui->titleEdit->clear();
        //this->ui->finishTime->setTime( this->ui->finishTime->minimumTime());
        //this->ui->membersInteger->setValue( EventTable::DefaultMembers );
        //this->ui->reviewerEdit->setText( Variable::instance()->string( "reviewerName" ));
    } else {
        const Row event = Event::instance()->indexToRow( EditorDialog::instance()->container->currentIndex().row());

        if ( event == Row::Invalid )
            return;

        this->ui->titleEdit->setText( Event::instance()->title( event ));
        //this->ui->finishTime->setTime( Team::instance()->finishTime( team ));
        //this->ui->membersInteger->setValue( Team::instance()->members( team ));
        //this->ui->reviewerEdit->setText( Team::instance()->reviewer( team ));
    }

    this->ui->titleEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}

