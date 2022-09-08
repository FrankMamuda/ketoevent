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
#include "main.h"
#include "task.h"
#include "taskedit.h"
#include "ui_taskedit.h"
#include <QCommonStyle>
#include <QMessageBox>
#include <QDebug>

// singleton
TaskEdit *TaskEdit::i = nullptr;

/**
 * @brief TaskEdit::TaskEdit
 * @param parent
 */
TaskEdit::TaskEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::TaskEdit ), m_edit( false ) {
    // set up defaults
    this->ui->setupUi( this );

    // setup pixmaps
    this->ui->namePixmap->setPixmap( QIcon::fromTheme( "name" ).pixmap( 16, 16 ));
    this->ui->multiPixmap->setPixmap( QIcon::fromTheme( "multi" ).pixmap( 16, 16 ));
    this->ui->stylePixmap->setPixmap( QIcon::fromTheme( "style" ).pixmap( 16, 16 ));
    this->ui->descPixmap->setPixmap( QIcon::fromTheme( "info" ).pixmap( 16, 16 ));
    this->ui->typePixmap->setPixmap( QIcon::fromTheme( "type" ).pixmap( 16, 16 ));
    this->ui->pointsPixmap->setPixmap( QIcon::fromTheme( "star" ).pixmap( 16, 16 ));

    // setup comboboxes
    this->connect<void( QComboBox::* )( int )>( this->ui->typeCombo, &QComboBox::activated, [ this ]( int index ) {
        const Task::Types type = static_cast<Task::Types>( index );

        if ( type == Task::Types::Check ) {
            this->ui->multiLabel->hide();
            this->ui->multiInteger->hide();
            this->ui->multiPixmap->hide();
        } else if ( type == Task::Types::Multi ) {
            this->ui->multiLabel->show();
            this->ui->multiInteger->show();
            this->ui->multiPixmap->show();
        }
    } );
    this->ui->typeCombo->addItems( Task::instance()->types.values());
    this->ui->styleCombo->addItems( Task::instance()->styles.values());

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
        if ( Task::instance()->contains( Task::Name, taskName ) && !this->isEditing()) {
            QMessageBox::information( this, this->tr( "Task already exists" ), this->tr( "Task already exists\nChoose a different name" ));
            return;
        }

        // if everything is ok, add a new task
        if ( !this->isEditing()) {
            Task::instance()->add( taskName, this->ui->pointsInteger->value(), this->ui->multiInteger->value(),
                                   static_cast<Task::Types>( this->ui->typeCombo->currentIndex()),
                                   static_cast<Task::Styles>( this->ui->styleCombo->currentIndex()),
                                   this->ui->descEdit->text());
        } else {
            const Row task = Task::instance()->row( EditorDialog::instance()->container->currentIndex().row());

            if ( task == Row::Invalid )
                return;

            Task::instance()->setName( task, taskName );
            Task::instance()->setPoints( task, this->ui->pointsInteger->value());
            Task::instance()->setMulti( task, this->ui->multiInteger->value());
            Task::instance()->setType( task, static_cast<Task::Types>( this->ui->typeCombo->currentIndex()));
            Task::instance()->setStyle( task, static_cast<Task::Styles>( this->ui->styleCombo->currentIndex()));
            Task::instance()->setDescription( task, this->ui->descEdit->text());
        }

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    });

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, []() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief TaskEdit::~TaskEdit
 */
TaskEdit::~TaskEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));

    // delete ui
    delete this->ui;
}

/**
 * @brief TaskEdit::reset
 */
void TaskEdit::reset( bool edit ) {
    this->m_edit = edit;

    if ( !this->isEditing()) {
        this->ui->nameEdit->clear();
        this->ui->pointsInteger->setValue( 1 );
        this->ui->multiInteger->setValue( this->ui->multiInteger->minimum());
        this->ui->typeCombo->setCurrentIndex( 0 );
        this->ui->styleCombo->setCurrentIndex( 0 );
        this->ui->descEdit->clear();
    } else {
        const Row task = Task::instance()->row( EditorDialog::instance()->container->currentIndex().row());

        if ( task == Row::Invalid )
            return;

        this->ui->nameEdit->setText( Task::instance()->name( task ));
        this->ui->pointsInteger->setValue( Task::instance()->points( task ));
        this->ui->multiInteger->setValue( Task::instance()->multi( task ));
        this->ui->typeCombo->setCurrentIndex( static_cast<int>( Task::instance()->type( task )));
        this->ui->styleCombo->setCurrentIndex( static_cast<int>( Task::instance()->style( task )));
        this->ui->descEdit->setText( Task::instance()->description( task ));
    }

    this->ui->nameEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}
