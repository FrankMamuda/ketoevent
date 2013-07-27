/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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
#include "gui_taskedit.h"
#include "main.h"
#include "ui_gui_taskedit.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

/*
================
construct
================
*/
Gui_TaskEdit::Gui_TaskEdit( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_TaskEdit ) {
    ui->setupUi( this );

    // set up view
    this->listModelPtr = new Gui_TaskListModel( this );
    this->ui->taskList->setModel( this->listModelPtr );
    this->ui->taskList->setAlternatingRowColors( true );

    // hide add/edit widget
    this->toggleAddEditWidget( NoState );

    // reset current match
    this->currentMatch = 0;

    // connect
    this->connect( this->ui->closeButton, SIGNAL( clicked()), this, SLOT( close()));
}

/*
================
destruct
================
*/
Gui_TaskEdit::~Gui_TaskEdit() {
    delete ui;
    delete this->listModelPtr;
}

/*
================
enableView
================
*/
void Gui_TaskEdit::enableView() {
    this->ui->addTaskButton->setEnabled( true );
    this->ui->removeTaskButton->setEnabled( true );
    this->ui->editTaskButton->setEnabled( true );
    this->ui->closeButton->setEnabled( true );
    this->ui->taskList->setEnabled( true );
    this->ui->taskList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->closeButton->setDefault( true );
    this->ui->findTask->setEnabled( true );
    this->ui->clearText->setEnabled( true );
}

/*
================
toggleAddEditWidget
================
*/
void Gui_TaskEdit::toggleAddEditWidget( AddEditState state ) {
    this->setState( state );

    if ( !this->ui->addEditWidget->isHidden()) {
        this->ui->addEditWidget->close();
        this->ui->taskList->setEnabled( true );
        this->enableView();
    } else {
        TaskEntry *taskPtr = NULL;

        // disable everything
        this->ui->addEditWidget->show();
        this->ui->addTaskButton->setDisabled( true );
        this->ui->removeTaskButton->setDisabled( true );
        this->ui->editTaskButton->setDisabled( true );
        this->ui->closeButton->setDisabled( true );
        this->ui->doneButton->setDefault( true );
        this->ui->findTask->setDisabled( true );
        this->ui->clearText->setDisabled( true );
        this->ui->taskList->setDisabled( true );
        this->ui->taskType->setCurrentIndex( 0 );
        this->changeTaskType( TaskEntry::Check );

        switch ( state ) {
        case Add:
            this->ui->taskName->clear();
            this->ui->taskPoints->setValue( 1 );
            this->ui->taskMaxMulti->clear();
            this->ui->taskChallenge->setChecked( false );
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add task" ));
            break;

        case Edit:
            // match by id
            taskPtr = m.taskForId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
            if ( taskPtr == NULL ) {
                this->toggleAddEditWidget( NoState );
                return;
            }

            this->ui->taskName->setText( taskPtr->name());
            this->ui->taskPoints->setValue( taskPtr->points());
            this->ui->taskType->setCurrentIndex( static_cast<int>( taskPtr->type()));
            this->ui->taskMaxMulti->setValue( taskPtr->multi());
            this->ui->taskChallenge->setChecked( taskPtr->isChallenge());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit task" ));

            if ( taskPtr->type() == TaskEntry::Check || taskPtr->type() == TaskEntry::Special ) {
                this->ui->taskMaxMulti->setValue( 0 );
                this->ui->taskMaxMulti->setDisabled( true );
            }

            if ( taskPtr->type() == TaskEntry::Special )
                this->ui->taskChallenge->setDisabled( true );

            break;

            // bad state
        case NoState:
        default:
            return;
        }
    }
}

/*
================
removeTaskButton->clicked
================
*/
void Gui_TaskEdit::on_removeTaskButton_clicked() {
    QSqlQuery query;
    TaskEntry *taskPtr = m.taskForId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
    int y = 1;

    if ( taskPtr == NULL ) {
        this->toggleAddEditWidget( NoState );
        return;
    }

    // begin reset
    this->listModelPtr->beginReset();

    // remove from internal list
    m.taskList.removeOne( taskPtr );

    // remove from database
    query.exec( QString( "delete from tasks where id=%1" ).arg( taskPtr->id()));

    // reindex database (yes, it is very unfortunate)
    foreach ( TaskEntry *reorderPtr, m.taskList ) {
        reorderPtr->setOrder( y );
        y++;
    }

    // end reset
    this->listModelPtr->endReset();
}

/*
================
doneButton->clicked
================
*/
void Gui_TaskEdit::on_doneButton_clicked() {
    TaskEntry *taskPtr = NULL;

    // failsafe
    if ( this->ui->taskName->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Please specify task name" ));
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
        return;
    }

    // begin reset
    this->listModelPtr->beginReset();

    // alternate between Add/Edit states
    if ( this->state() == Add ) {
        m.addTask( this->ui->taskName->text(), this->ui->taskPoints->value(), this->ui->taskMaxMulti->value(), this->ui->taskChallenge->isChecked(), static_cast<TaskEntry::Types>( this->ui->taskType->currentIndex()));
    } else if ( this->state() == Edit ) {
        // match by id
        taskPtr = m.taskForId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());;

        if ( taskPtr == NULL ) {
            this->toggleAddEditWidget( NoState );
            return;
        }

        // set edited data
        taskPtr->setName( this->ui->taskName->text());
        taskPtr->setPoints( this->ui->taskPoints->value());
        taskPtr->setMulti( this->ui->taskMaxMulti->value());
        taskPtr->setChallenge( this->ui->taskChallenge->isChecked());
        taskPtr->setType( static_cast<TaskEntry::Types>( this->ui->taskType->currentIndex()));;
    }

    // reset view
    this->toggleAddEditWidget( NoState );
    this->currentMatch = 0;
    this->listModelPtr->endReset();

    // TODO: select last added/edited value (not implemented)
    //index = this->listModelPtr->index( m.taskList.indexOf( entryPtr ), 0 );
    //this->ui->taskList->setCurrentIndex( index );
}

/*
================
keyPressEvent
================
*/
void Gui_TaskEdit::keyPressEvent( QKeyEvent *ePtr ) {
    // ignore close button to search for a task
    if ( ePtr->key() == Qt::Key_Return && this->ui->findTask->hasFocus()) {
        this->findTask();
        return;
    }
    QDialog::keyPressEvent( ePtr );
}

/*
================
findTask
================
*/
void Gui_TaskEdit::findTask() {
    int y;
    QString matchString;
    QModelIndex index;
    bool match = false;

    matchString = this->ui->findTask->text();

    // failsafe
    if ( matchString.isEmpty())
        return;

    // advance
    if ( this->currentMatch >= this->listModelPtr->rowCount() - 1 || this->currentMatch <= 0 )
        this->currentMatch = 0;
    else
        this->currentMatch++;

    // find item from current position
    for ( y = this->currentMatch; y < this->listModelPtr->rowCount(); y++ ) {
        index = this->listModelPtr->index( y, 0 );
        // list must be the same as in App_Main, don't match by display role
        if ( index.isValid()) {
            if ( m.taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                currentMatch = y;
                break;
            }
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->listModelPtr->rowCount(); y++ ) {
            index = this->listModelPtr->index( y, 0 );
            if ( index.isValid()) {
                // list must be the same as in App_Main, don't match by display role
                if ( m.taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
                    match = true;
                    currentMatch = y;
                    break;
                }
            }
        }
    }

    // matched?
    if ( match ) {
        this->ui->taskList->setCurrentIndex( index );
    } else {
        this->ui->taskList->clearSelection();
        QPalette p( this->ui->findTask->palette());
        p.setColor( QPalette::Base, QColor( 255, 0, 0, 64 ));
        this->ui->findTask->setPalette( p );
    }
}

/*
================
clearText->clicked
================
*/
void Gui_TaskEdit::on_clearText_clicked() {
    this->ui->findTask->clear();
}

/*
================
findTask->textChanged
================
*/
void Gui_TaskEdit::on_findTask_textChanged( const QString & ) {
    if ( this->ui->findTask->palette().color( QPalette::Base ) == QColor( 255, 0, 0, 64 )) {
        QPalette p( this->ui->findTask->palette());
        p.setColor( QPalette::Base, Qt::white );
        this->ui->findTask->setPalette( p );
    }
}

/*
================
changeTaskType
================
*/
void Gui_TaskEdit::changeTaskType( TaskEntry::Types type ) {
    switch ( type ) {
    case TaskEntry::Check:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setDisabled( true );
        this->ui->taskChallenge->setEnabled( true );
        break;

    case TaskEntry::Multi:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setEnabled( true );
        this->ui->taskChallenge->setEnabled( true );
        break;

    case TaskEntry::Special:
        this->ui->taskPoints->setDisabled( true );
        this->ui->taskMaxMulti->setDisabled( true );
        this->ui->taskChallenge->setDisabled( true );
        break;

    default:
        return;
    }
}

/*
================
taskType->currentIndexChanged
================
*/
void Gui_TaskEdit::on_taskType_currentIndexChanged( int index ) {
    TaskEntry::Types type = static_cast<TaskEntry::Types>( index );
    this->changeTaskType( type );
}

/*
================
move
================
*/
void Gui_TaskEdit::move( MoveDirection direction ) {
    TaskEntry *taskPtr = m.taskForId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
    QModelIndex index;
    int y, t0, t1;
    int k = 0;

    // begin reset
    this->listModelPtr->beginReset();

    // match name to be sure
    if ( taskPtr == NULL )
        return;

    y = m.taskList.indexOf( taskPtr );

    if ( direction == Up && y != 0 ) {
        k = y - 1;

        // move in database
        t0 = m.taskList.at( y )->order();
        t1 = m.taskList.at( k )->order();
        m.taskList.at( y )->setOrder( t1 );
        m.taskList.at( k )->setOrder( t0 );

        // move in memory
        m.taskList.move( y, k );
    } else if ( direction == Down && y != m.taskList.count() - 1 ) {
        k = y + 1;

        // move in database
        t0 = m.taskList.at( y )->order();
        t1 = m.taskList.at( k )->order();
        m.taskList.at( y )->setOrder( t1 );
        m.taskList.at( k )->setOrder( t0 );

        // move in memory
        m.taskList.move( y, k );
    }

    // end reset
    this->listModelPtr->endReset();

    // reselect value
    index = this->listModelPtr->index( k, 0 );
    this->ui->taskList->setCurrentIndex( index );
    this->currentMatch = k;
}

/*
================
upButton->clicked
================
*/
void Gui_TaskEdit::on_upButton_clicked() {
    this->move( Up );
}

/*
================
downButton->clicked
================
*/
void Gui_TaskEdit::on_downButton_clicked() {
    this->move( Down );
}

