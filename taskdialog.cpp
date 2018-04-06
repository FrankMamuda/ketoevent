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
#include "taskdialog.h"
#include "mainwindow.h"
#include "main.h"
#include "ui_taskdialog.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QTimer>

/**
 * @brief TaskDialog::TaskDialog
 * @param parent
 */
TaskDialog::TaskDialog( QWidget *parent ) : Dialog( parent ), ui( new Ui::TaskDialog ) {
    this->ui->setupUi( this );

    // set up view
    this->taskModel = new TaskListModel( this );
    this->ui->taskList->setModel( this->taskModel );
    this->ui->taskList->setAlternatingRowColors( true );
    this->ui->taskList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->taskMaxMulti->setMinimum( 2 );

    // hide add/edit widget
    this->toggleAddEditWidget( NoState );

    // reset current match
    this->setCurrentMatch();

    // connect
    this->connect( this->ui->closeButton, SIGNAL( clicked()), this, SLOT( close()));

    // disable up/down buttons
    this->ui->actionMoveUp->setDisabled( true );
    this->ui->actionMoveDown->setDisabled( true );

    // set focus
    this->ui->closeButton->setFocus();

    // jump to points on enter
    this->connect( this->ui->taskName, &QLineEdit::returnPressed, [ this ] {
        QTimer::singleShot( 0, this->ui->taskPoints, SLOT( setFocus()));
        this->ui->taskPoints->selectAll();
    } );

    // jump to close on next enter
    this->ui->taskPoints->setKeyboardTracking( false );
    this->connect( this->ui->taskPoints, &QSpinBox::editingFinished, [ this ] {
        QTimer::singleShot( 0, this->ui->doneButton, SLOT( setFocus()));
    } );

    // jump from desc
    this->connect( this->ui->taskDescription, &QLineEdit::returnPressed, [ this ] {
        QTimer::singleShot( 0, this->ui->doneButton, SLOT( setFocus()));
    } );
}

/**
 * @brief TaskDialog::~TaskDialog
 */
TaskDialog::~TaskDialog() {
    delete ui;
    delete this->taskModel;
}

/**
 * @brief TaskDialog::toggleView
 * @param viewState
 */
void TaskDialog::toggleView( ViewState viewState ) {
    bool state = false;

    if ( viewState == Enabled )
        state = true;

    this->ui->actionAdd->setEnabled( state );
    this->ui->actionRemove->setEnabled( state );
    this->ui->actionEdit->setEnabled( state );
    this->ui->actionSort->setEnabled( state );
    this->ui->actionMoveUp->setEnabled( state );
    this->ui->actionMoveDown->setEnabled( state );
    this->ui->closeButton->setEnabled( state );
    this->ui->taskList->setEnabled( state );
    this->ui->closeButton->setDefault( state );
    this->ui->findTask->setEnabled( state );
    this->ui->clearText->setEnabled( state );
    this->ui->taskDescription->setDisabled( state );

    if ( !state )
        QTimer::singleShot( 0, this->ui->taskName, SLOT( setFocus()));

    this->ui->taskList->scrollToBottom();
}

/**
 * @brief TaskDialog::toggleAddEditWidget
 * @param state
 */
void TaskDialog::toggleAddEditWidget( AddEditState state ) {
    this->setState( state );

    if ( !this->ui->addEditWidget->isHidden() || state == NoState ) {
        this->ui->addEditWidget->close();
        this->ui->taskList->setEnabled( true );
        this->toggleView();
    } else {
        Task *task = nullptr;

        // disable everything
        this->ui->addEditWidget->show();
        this->toggleView( Disabled );
        this->ui->taskType->setCurrentIndex( 0 );
        this->changeTaskType( Task::Check );
        this->ui->doneButton->setEnabled( true );
        this->ui->taskDescription->setEnabled( true );

        // refocus
        this->ui->doneButton->setFocus();

        switch ( state ) {
        case Add:
            this->ui->taskName->clear();
            this->ui->taskPoints->setValue( 1 );
            this->ui->taskMaxMulti->clear();
            this->ui->taskDescription->clear();
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add task" ));
            this->ui->taskStyle->setCurrentIndex( 0 );
            break;

        case Edit:
            // match by id
            task = Task::forId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
            if ( task == nullptr ) {
                this->toggleAddEditWidget( NoState );
                return;
            }

            this->ui->taskName->setText( task->name());
            this->ui->taskDescription->setText( task->description());
            this->ui->taskPoints->setValue( task->points());
            this->ui->taskType->setCurrentIndex( static_cast<int>( task->type()));
            this->ui->taskStyle->setCurrentIndex( static_cast<int>( task->style()));
            this->ui->taskMaxMulti->setValue( task->multi());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit task" ));

            if ( task->type() == Task::Check ) {
                this->ui->taskMaxMulti->setValue( 2 );
                this->ui->taskMaxMulti->setDisabled( true );
            }

            break;

            // bad state
        case NoState:
        default:
            // set focus
            this->ui->closeButton->setFocus();

            return;
        }
    }
}

/**
 * @brief TaskDialog::on_doneButton_clicked
 */
void TaskDialog::on_doneButton_clicked() {
    Task *task = nullptr;
    QModelIndex lastIndex;

    // failsafe
    if ( this->ui->taskName->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Please specify task name" ));
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
        return;
    }

    // begin reset
    this->taskModel->beginReset();

    // alternate between Add/Edit states
    if ( this->state() == Add ) {
        Task::add( this->ui->taskName->text(), this->ui->taskPoints->value(), this->ui->taskMaxMulti->value(), static_cast<Task::Types>( this->ui->taskType->currentIndex()), static_cast<Task::Styles>( this->ui->taskStyle->currentIndex()), this->ui->taskDescription->text());
        lastIndex = this->taskModel->index( this->taskModel->rowCount()-1);
    } else if ( this->state() == Edit ) {
        // match by id
        task = Task::forId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());;

        if ( task == nullptr ) {
            this->toggleAddEditWidget( NoState );
            return;
        }

        // set edited data
        task->setName( this->ui->taskName->text());
        task->setPoints( this->ui->taskPoints->value());
        task->setMulti( this->ui->taskMaxMulti->value());
        task->setType( static_cast<Task::Types>( this->ui->taskType->currentIndex()));;
        task->setStyle( static_cast<Task::Styles>( this->ui->taskStyle->currentIndex()));;
        task->setDescription( this->ui->taskDescription->text());

        // get last index
        lastIndex = this->ui->taskList->currentIndex();
    }

    // reset view
    this->toggleAddEditWidget( NoState );
    this->setCurrentMatch();
    this->taskModel->endReset();

    // select last added/edited value
    this->ui->taskList->setCurrentIndex( lastIndex );

    // reset this
    this->changeUpDownState( this->ui->taskList->currentIndex());
}

/**
 * @brief TaskDialog::keyPressEvent
 * @param ePtr
 */
void TaskDialog::keyPressEvent( QKeyEvent *ePtr ) {
    // ignore close button to search for a task
    if ( ePtr->key() == Qt::Key_Return && this->ui->findTask->hasFocus()) {
        this->findTask();
        return;
    }
    Dialog::keyPressEvent( ePtr );
}

/**
 * @brief TaskDialog::findTask
 */
void TaskDialog::findTask() {
    int y;
    QString matchString;
    QModelIndex index;
    bool match = false;

    // TODO: reuse code from MainWindow?

    matchString = this->ui->findTask->text();

    // failsafe
    if ( matchString.isEmpty())
        return;

    // advance
    if ( this->currentMatch() >= this->taskModel->rowCount() - 1 || this->currentMatch() <= 0 )
        this->setCurrentMatch();
    else
        this->setCurrentMatch( this->currentMatch() + 1 );

    // find item from current position
    for ( y = this->currentMatch(); y < this->taskModel->rowCount(); y++ ) {
        index = this->taskModel->index( y, 0 );
        // list must be the same as in App_Main, don't match by display role
        if ( index.isValid()) {
            if ( EventManager::instance()->active()->taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
                match = true;
                this->setCurrentMatch( y );
                break;
            }
        }
    }

    // no match, try again from beginning
    if ( !match ) {
        for ( y = 0; y < this->taskModel->rowCount(); y++ ) {
            index = this->taskModel->index( y, 0 );
            if ( index.isValid()) {
                // list must be the same as in App_Main, don't match by display role
                if ( EventManager::instance()->active()->taskList.at( index.row())->name().contains( matchString, Qt::CaseInsensitive )) {
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

    // reset this
    this->changeUpDownState( this->ui->taskList->currentIndex());
}

/**
 * @brief TaskDialog::on_clearText_clicked
 */
void TaskDialog::on_clearText_clicked() {
    this->ui->findTask->clear();
}

/**
 * @brief TaskDialog::on_findTask_textChanged
 */
void TaskDialog::on_findTask_textChanged( const QString & ) {
    if ( this->ui->findTask->palette().color( QPalette::Base ) == QColor( 255, 0, 0, 64 )) {
        QPalette p( this->ui->findTask->palette());
        p.setColor( QPalette::Base, Qt::white );
        this->ui->findTask->setPalette( p );
    }
}

/**
 * @brief TaskDialog::changeTaskType
 * @param type
 */
void TaskDialog::changeTaskType( Task::Types type ) {
    switch ( type ) {
    case Task::Check:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setDisabled( true );
        break;

    case Task::Multi:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setEnabled( true );
        break;

    default:
        return;
    }
}

/**
 * @brief TaskDialog::on_taskType_currentIndexChanged
 * @param index
 */
void TaskDialog::on_taskType_currentIndexChanged( int index ) {
    Task::Types type = static_cast<Task::Types>( index );
    this->changeTaskType( type );
}

/**
 * @brief TaskDialog::move
 * @param direction
 */
void TaskDialog::move( MoveDirection direction ) {
    Task *task = Task::forId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
    QModelIndex index;
    int y, t0, t1;
    int k = 0;

    // begin reset
    this->taskModel->beginReset();

    // match name to be sure
    if ( task == nullptr )
        return;

    y = EventManager::instance()->active()->taskList.indexOf( task );

    if ( direction == Up && y != 0 ) {
        k = y - 1;

        // move in database
        t0 = EventManager::instance()->active()->taskList.at( y )->order();
        t1 = EventManager::instance()->active()->taskList.at( k )->order();
        EventManager::instance()->active()->taskList.at( y )->setOrder( t1 );
        EventManager::instance()->active()->taskList.at( k )->setOrder( t0 );

        // move in memory
        EventManager::instance()->active()->taskList.move( y, k );
    } else if ( direction == Down && y != EventManager::instance()->active()->taskList.count() - 1 ) {
        k = y + 1;

        // move in database
        t0 = EventManager::instance()->active()->taskList.at( y )->order();
        t1 = EventManager::instance()->active()->taskList.at( k )->order();
        EventManager::instance()->active()->taskList.at( y )->setOrder( t1 );
        EventManager::instance()->active()->taskList.at( k )->setOrder( t0 );

        // move in memory
        EventManager::instance()->active()->taskList.move( y, k );
    }

    // end reset
    this->taskModel->endReset();

    // reselect value
    index = this->taskModel->index( k, 0 );
    this->ui->taskList->setCurrentIndex( index );
    this->setCurrentMatch( k );

    // check the buttons!
    this->changeUpDownState( this->ui->taskList->currentIndex());
}

/**
 * @brief TaskDialog::on_actionRemove_triggered
 */
void TaskDialog::on_actionRemove_triggered() {
    QSqlQuery query;
    Task *task = Task::forId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
    int y = 1;

    if ( task == nullptr ) {
        this->toggleAddEditWidget( NoState );
        return;
    }

    // begin reset
    this->taskModel->beginReset();

    // remove from internal list (both base and current event - there should be
    //   no mismatches, since eventId differs for tasks)
    EventManager::instance()->active()->taskList.removeOne( task );
    Main::instance()->taskList.removeOne( task );

    // remove from database
    query.exec( QString( "delete from tasks where id=%1" ).arg( task->id()));

    // database reindexing must be performed, however this is done in-memory,
    // actual changes are written to disk only when requested
    foreach ( Task *reorderPtr, EventManager::instance()->active()->taskList ) {
        reorderPtr->setOrder( y );
        y++;
    }

    // end reset
    this->taskModel->endReset();

    // reset this
    this->changeUpDownState( this->ui->taskList->currentIndex());
}

/**
 * @brief TaskDialog::on_actionSort_triggered
 */
void TaskDialog::on_actionSort_triggered() {
    int y = 0;

    // begin reset
    this->taskModel->beginReset();

    // sort by name
    Main::instance()->sort( Main::Tasks );

    // reindex whole list
    foreach ( Task *task, EventManager::instance()->active()->taskList ) {
        task->setOrder( y );
        y++;
    }

    // end reset
    this->taskModel->endReset();
}

/**
 * @brief TaskDialog::closeEvent
 * @param ePtr
 */
void TaskDialog::closeEvent( QCloseEvent *ePtr ) {
    MainWindow *mainWindow;

    this->toggleAddEditWidget( NoState );
    ePtr->accept();

    // reindex tasks here
    Database::reindexTasks();

    // refill tasks in gui
    mainWindow = qobject_cast<MainWindow*>( this->parent());
    if ( mainWindow != nullptr )
        mainWindow->fillTasks();
}

/**
 * @brief TaskDialog::changeUpDownState
 * @param index
 */
void TaskDialog::changeUpDownState( const QModelIndex &index ) {
    Task *task = Task::forId( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::UserRole ).toInt());
    if ( task == nullptr ) {
        this->ui->actionMoveUp->setDisabled( true );
        this->ui->actionMoveDown->setDisabled( true );
        return;
    }

    if ( EventManager::instance()->active()->taskList.count() > 1 && index.row() == 0 ) {
        this->ui->actionMoveUp->setDisabled( true );
        this->ui->actionMoveDown->setEnabled( true );
        return;
    }

    if ( index.row() > 0 && index.row() == EventManager::instance()->active()->taskList.count() - 1 ) {
        this->ui->actionMoveUp->setEnabled( true );
        this->ui->actionMoveDown->setDisabled( true );
        return;
    }

    this->ui->actionMoveUp->setEnabled( true );
    this->ui->actionMoveDown->setEnabled( true );
}

/**
 * @brief TaskDialog::on_taskList_clicked
 * @param index
 */
void TaskDialog::on_taskList_clicked( const QModelIndex &index ) {
    this->changeUpDownState( index );
}
