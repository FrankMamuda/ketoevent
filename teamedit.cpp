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
#include "team.h"
#include "teamedit.h"
#include "ui_teamedit.h"
#include <QCommonStyle>
#include <QMessageBox>

/**
 * @brief TeamEdit::TeamEdit
 * @param parent
 */
TeamEdit::TeamEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::TeamEdit ) {
    QCommonStyle style;

    // set up defaults
    this->ui->setupUi( this );
    this->ui->membersInteger->setMinimum( EventTable::DefaultMinMembers );
    this->ui->membersInteger->setMaximum( EventTable::DefaultMaxMembers );
    this->ui->finishTime->setMinimumTime( QTime::fromString( EventTable::DefaultStartTime, "hh:mm" ));
    this->ui->finishTime->setMaximumTime( QTime::fromString( EventTable::DefaultFinalTime, "hh:mm" ));
    this->ui->addButton->setIcon( style.standardIcon( QStyle::SP_DialogOkButton ));
    this->ui->cancelButton->setIcon( style.standardIcon( QStyle::SP_DialogCancelButton ));

    // empty team title check
    auto emptyTitle = [ this ]() {
        // warn upon empty team title
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::information( this, this->tr( "Empty team title" ), this->tr( "Please enter team title" ));
            return true;
        }
        return false;
    };

    // add button action
    this->connect( this->ui->addButton, &QPushButton::clicked, [ this, emptyTitle ]() {
        const QString teamTitle( this->ui->titleEdit->text());

        // abort on empty team title
        if ( emptyTitle())
            return;

        // abort on existing team
        if ( Team::instance()->contains( Team::Title, teamTitle )) {
            QMessageBox::information( this, this->tr( "Team already exists" ), this->tr( "Team already exists\nChoose a different title" ));
            return;
        }

        // if everything is ok, add a new team
        Team::instance()->add( teamTitle, this->ui->membersInteger->value(), this->ui->finishTime->time(), this->ui->reviewerEdit->text());

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    });

    // shortcut from title to members
    this->connect( this->ui->titleEdit, &QLineEdit::returnPressed, [ this, emptyTitle ]() {
        if ( emptyTitle())
            return;

        this->ui->membersInteger->setFocus();
    } );

    // shortcut from members to time
    this->connect( this->ui->membersInteger, &QSpinBox::editingFinished, [ this ]() {
        this->ui->finishTime->setFocus();
    } );

    // shortcut from time to add button
    this->connect( this->ui->finishTime, &QTimeEdit::editingFinished, [ this ]() {
        this->ui->addButton->setFocus();
        this->ui->addButton->setDefault( true );
        this->ui->addButton->setAutoDefault( true );
    } );

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, [ this ]() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    });
}

/**
 * @brief TeamEdit::~TeamEdit
 */
TeamEdit::~TeamEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));
    this->disconnect( this->ui->titleEdit, SIGNAL( returnPressed()));
    this->disconnect( this->ui->membersInteger, SIGNAL( editingFinished()));
    this->disconnect( this->ui->finishTime, SIGNAL( editingFinished()));

    // delete ui
    delete this->ui;
}

/**
 * @brief TeamEdit::reset
 */
void TeamEdit::reset() {
    // reset ui components to default values
    this->ui->titleEdit->clear();
    this->ui->finishTime->setTime( this->ui->finishTime->minimumTime());
    this->ui->membersInteger->setValue( EventTable::DefaultMembers );

    // TODO: set default reviewer
    //this->ui->reviewerEdit->setText()
    this->ui->titleEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}
