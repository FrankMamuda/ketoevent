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
#include "task.h"
#include "taskedit.h"
#include "ui_taskedit.h"
#include <QCommonStyle>
#include <QMessageBox>

/**
 * @brief TaskEdit::TaskEdit
 * @param parent
 */
TaskEdit::TaskEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::TaskEdit ) {
    QCommonStyle style;

    // set up defaults
    this->ui->setupUi( this );
    this->ui->addButton->setIcon( style.standardIcon( QStyle::SP_DialogOkButton ));
    this->ui->cancelButton->setIcon( style.standardIcon( QStyle::SP_DialogCancelButton ));

    // empty task name check
    auto emptyName = [ this ]() {
        // warn upon empty task name
        if ( this->ui->nameEdit->text().isEmpty()) {
            QMessageBox::information( this, this->tr( "Empty task name" ), this->tr( "Please enter task name" ));
            return true;
        }
        return false;
    };

    // add button action
    this->connect( this->ui->addButton, &QPushButton::clicked, [ this, emptyName ]() {
        const QString taskName( this->ui->nameEdit->text());

        // abort on empty task name
        if ( emptyName())
            return;

        // abort on existing task
        if ( Task::instance()->contains( Task::Name, taskName )) {
            QMessageBox::information( this, this->tr( "Task already exists" ), this->tr( "Task already exists\nChoose a different name" ));
            return;
        }

        // if everything is ok, add a new task
        //Task::instance()->add( taskName, this->ui->membersInteger->value(), this->ui->finishTime->time(), this->ui->reviewerEdit->text());

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    });

#if 0
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
#endif

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, [ this ]() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    });
}

/**
 * @brief TaskEdit::~TaskEdit
 */
TaskEdit::~TaskEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));
#if 0
    this->disconnect( this->ui->titleEdit, SIGNAL( returnPressed()));
    this->disconnect( this->ui->membersInteger, SIGNAL( editingFinished()));
    this->disconnect( this->ui->finishTime, SIGNAL( editingFinished()));
#endif

    // delete ui
    delete this->ui;
}

/**
 * @brief TaskEdit::reset
 */
void TaskEdit::reset() {
#if 0
    // reset ui components to default values
    this->ui->titleEdit->clear();
    this->ui->finishTime->setTime( this->ui->finishTime->minimumTime());
    this->ui->membersInteger->setValue( EventTable::DefaultMembers );

    // TODO: set default reviewer
    //this->ui->reviewerEdit->setText()
#endif
    this->ui->nameEdit->setFocus();

    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}
