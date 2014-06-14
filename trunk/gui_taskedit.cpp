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
#include "gui_taskedit.h"
#include "gui_main.h"
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
    this->ui->taskList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->taskMaxMulti->setMinimum( 2 );

    // hide add/edit widget
    this->toggleAddEditWidget( NoState );

    // reset current match
    this->setCurrentMatch();

    // connect
    this->connect( this->ui->closeButton, SIGNAL( clicked()), this, SLOT( close()));

#ifdef Q_OS_ANDROID
    // android fullscreen fix
    QWidget *wPtr = qobject_cast<QWidget*>( this->parent());
    this->setGeometry( wPtr->geometry());
#endif
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
toggleView
================
*/
void Gui_TaskEdit::toggleView( ViewState viewState ) {
    bool state = false;

    if ( viewState == Enabled )
        state = true;

    this->ui->addTaskButton->setEnabled( state );
    this->ui->removeTaskButton->setEnabled( state );
    this->ui->editTaskButton->setEnabled( state );
    this->ui->sortButton->setEnabled( state );
    this->ui->closeButton->setEnabled( state );
    this->ui->taskList->setEnabled( state );
    this->ui->closeButton->setDefault( state );
    this->ui->findTask->setEnabled( state );
    this->ui->clearText->setEnabled( state );
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
        this->toggleView();
    } else {
        TaskEntry *taskPtr = NULL;

        // disable everything
        this->ui->addEditWidget->show();
        this->toggleView( Disabled );
        this->ui->taskType->setCurrentIndex( 0 );
        this->changeTaskType( TaskEntry::Check );
        this->ui->doneButton->setEnabled( true );

        switch ( state ) {
        case Add:
            this->ui->taskName->clear();
            this->ui->taskPoints->setValue( 1 );
            this->ui->taskMaxMulti->clear();
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add task" ));
            this->ui->taskStyle->setCurrentIndex( 0 );
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
            this->ui->taskStyle->setCurrentIndex( static_cast<int>( taskPtr->style()));
            this->ui->taskMaxMulti->setValue( taskPtr->multi());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit task" ));

            if ( taskPtr->type() == TaskEntry::Check ) {
                this->ui->taskMaxMulti->setValue( 2 );
                this->ui->taskMaxMulti->setDisabled( true );
            }

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
    m.base.taskList.removeOne( taskPtr );

    // remove from database
    query.exec( QString( "delete from tasks where id=%1" ).arg( taskPtr->id()));

    // reindex database (yes, it is very unfortunate)
    foreach ( TaskEntry *reorderPtr, m.base.taskList ) {
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
#ifdef Q_OS_ANDROID
        msgBox.setGeometry( this->geometry());
#endif
        return;
    }

    // begin reset
    this->listModelPtr->beginReset();

    // alternate between Add/Edit states
    if ( this->state() == Add ) {
        m.addTask( this->ui->taskName->text(), this->ui->taskPoints->value(), this->ui->taskMaxMulti->value(), static_cast<TaskEntry::Types>( this->ui->taskType->currentIndex()), static_cast<TaskEntry::Styles>( this->ui->taskStyle->currentIndex()));
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
        taskPtr->setType( static_cast<TaskEntry::Types>( this->ui->taskType->currentIndex()));;
        taskPtr->setStyle( static_cast<TaskEntry::Styles>( this->ui->taskStyle->currentIndex()));;
    }

    // reset view
    this->toggleAddEditWidget( NoState );
    this->setCurrentMatch();
    this->listModelPtr->endReset();

    // TODO: select last added/edited value (not implemented)
    //index = this->listModelPtr->index( m.base.taskList.indexOf( entryPtr ), 0 );
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

  reuse code from Gui_Main?
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
    if ( this->currentMatch() >= this->listModelPtr->rowCount() - 1 || this->currentMatch() <= 0 )
        this->setCurrentMatch();
    else
        this->setCurrentMatch( this->currentMatch() + 1 );

    // find item from current position
    for ( y = this->currentMatch(); y < this->listModelPtr->rowCount(); y++ ) {
        index = this->listModelPtr->index( y, 0 );
        // list must be the same as in App_Main, don't match by display role
        if ( index.isValid()) {
            if ( m.base.taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                this->setCurrentMatch( y );
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
                if ( m.base.taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
                    match = true;
                    this->setCurrentMatch( y );
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
        break;

    case TaskEntry::Multi:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setEnabled( true );
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

    y = m.base.taskList.indexOf( taskPtr );

    if ( direction == Up && y != 0 ) {
        k = y - 1;

        // move in database
        t0 = m.base.taskList.at( y )->order();
        t1 = m.base.taskList.at( k )->order();
        m.base.taskList.at( y )->setOrder( t1 );
        m.base.taskList.at( k )->setOrder( t0 );

        // move in memory
        m.base.taskList.move( y, k );
    } else if ( direction == Down && y != m.base.taskList.count() - 1 ) {
        k = y + 1;

        // move in database
        t0 = m.base.taskList.at( y )->order();
        t1 = m.base.taskList.at( k )->order();
        m.base.taskList.at( y )->setOrder( t1 );
        m.base.taskList.at( k )->setOrder( t0 );

        // move in memory
        m.base.taskList.move( y, k );
    }

    // end reset
    this->listModelPtr->endReset();

    // reselect value
    index = this->listModelPtr->index( k, 0 );
    this->ui->taskList->setCurrentIndex( index );
    this->setCurrentMatch( k );
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

/*
================
sortButton->clicked
================
*/
void Gui_TaskEdit::on_sortButton_clicked() {
    int y = 0;

    // begin reset
    this->listModelPtr->beginReset();

    // sort by name
    m.sort( Main::Tasks );

    // reindex whole list
    foreach ( TaskEntry *taskPtr, m.base.taskList ) {
        taskPtr->setOrder( y );
        y++;
    }

    // end reset
    this->listModelPtr->endReset();
}
