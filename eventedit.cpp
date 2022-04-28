/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "eventedit.h"
#include "ui_eventedit.h"
#include "variable.h"
#include <QMessageBox>

// singleton
EventEdit *EventEdit::i = nullptr;

/**
 * @brief EventEdit::EventEdit
 * @param parent
 */
EventEdit::EventEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::EventEdit ), m_edit( false ) {
    // set up ui
    this->ui->setupUi( this );

    // setup pixmaps
    this->ui->titlePixmap->setPixmap( QIcon::fromTheme( "name" ).pixmap( 16, 16 ));

    // only visible in quick add
    this->setWindowTitle( this->tr( "Add event" ));

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
        Row event = Row::Invalid;
        if ( !this->isEditing()) {
            event = Event::instance()->add( eventTitle,
                                    this->ui->minInteger->value(),
                                    this->ui->maxInteger->value(),
                                    this->ui->startTime->time(),
                                    this->ui->finishTime->time(),
                                    this->ui->finalTime->time(),
                                    this->ui->penaltyInteger->value(),
                                    this->ui->twoInteger->value(),
                                    this->ui->threeInteger->value(),
                                    this->ui->fourPlusInteger->value());
        } else {
            const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

            if ( event == Row::Invalid )
                return;

            Event::instance()->setTitle( event, eventTitle );
            Event::instance()->setMinMembers( event, this->ui->minInteger->value());
            Event::instance()->setMaxMembers( event, this->ui->maxInteger->value());
            Event::instance()->setStartTime( event, this->ui->startTime->time());
            Event::instance()->setFinishTime( event, this->ui->finishTime->time());
            Event::instance()->setFinalTime( event, this->ui->finalTime->time());
            Event::instance()->setPenaltyPoints( event, this->ui->penaltyInteger->value());
            Event::instance()->setComboOfTwo( event, this->ui->twoInteger->value());
            Event::instance()->setComboOfThree( event, this->ui->threeInteger->value());
            Event::instance()->setComboOfFourPlus( event, this->ui->fourPlusInteger->value());
        }

        if ( event != Row::Invalid )
            MainWindow::instance()->setCurrentEvent( event );

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
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
        this->ui->minInteger->setValue( EventTable::DefaultMinMembers );
        this->ui->maxInteger->setValue( EventTable::DefaultMaxMembers );
        this->ui->startTime->setTime( QTime::fromString( EventTable::DefaultStartTime, Database_::TimeFormat ));
        this->ui->finishTime->setTime( QTime::fromString( EventTable::DefaultFinishTime, Database_::TimeFormat ));
        this->ui->finalTime->setTime( QTime::fromString( EventTable::DefaultFinalTime, Database_::TimeFormat ));
        this->ui->penaltyInteger->setValue( EventTable::DefaultPenaltyPoints );
        this->ui->twoInteger->setValue( EventTable::DefaultComboOfTwo );
        this->ui->threeInteger->setValue( EventTable::DefaultComboOfThree );
        this->ui->fourPlusInteger->setValue( EventTable::DefaultComboOfFourAndMore );
    } else {
        const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

        if ( event == Row::Invalid )
            return;

        this->ui->titleEdit->setText( Event::instance()->title( event ));
        this->ui->minInteger->setValue( Event::instance()->minMembers( event ));
        this->ui->maxInteger->setValue( Event::instance()->maxMembers( event ));
        this->ui->startTime->setTime( Event::instance()->startTime( event ));
        this->ui->finishTime->setTime( Event::instance()->finishTime( event ));
        this->ui->finalTime->setTime( Event::instance()->finalTime( event ));
        this->ui->penaltyInteger->setValue( Event::instance()->penalty( event ));
        this->ui->twoInteger->setValue( Event::instance()->comboOfTwo( event ));
        this->ui->threeInteger->setValue( Event::instance()->comboOfThree( event ));
        this->ui->fourPlusInteger->setValue( Event::instance()->comboOfFourPlus( event ));
    }

    this->ui->titleEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}

