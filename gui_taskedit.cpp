
//
// includes
//
#include "gui_taskedit.h"
#include "app_taskentry.h"
#include "app_main.h"
#include "ui_gui_taskedit.h"
#include <QMessageBox>
#include "sys_filesystem.h"

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
        App_TaskEntry *entryPtr = NULL;

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
        this->changeTaskType( App_TaskEntry::Check );

        switch ( state ) {
        case Add:
            this->ui->taskName->clear();
            this->ui->taskPoints->setValue( 1 );
            this->ui->taskMaxMulti->clear();
            this->ui->taskChallenge->setChecked( false );
            this->ui->addEditWidget->setWindowTitle( this->tr( "Add task" ));
            break;

        case Edit:
            // match name to be sure
            entryPtr = m.findTaskByName( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::DisplayRole ).toString());
            if ( entryPtr == NULL ) {
                this->toggleAddEditWidget( NoState );
                return;
            }

            this->ui->taskName->setText( entryPtr->name());
            this->ui->taskPoints->setValue( entryPtr->points());
            this->ui->taskType->setCurrentIndex( static_cast<int>( entryPtr->type()));
            this->ui->taskMaxMulti->setValue( entryPtr->maxMulti());
            this->ui->taskChallenge->setChecked( entryPtr->isChallenge());
            this->ui->addEditWidget->setWindowTitle( this->tr( "Edit task" ));

            /*if ( entryPtr->type() == App_TaskEntry::Check || entryPtr->type() == App_TaskEntry::Special ) {
                this->ui->taskMaxMulti->setValue( 0 );
                this->ui->taskMaxMulti->setDisabled( true );
            }

            if ( entryPtr->type() == App_TaskEntry::Special )
                this->ui->taskChallenge->setDisabled( true );*/

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
    int state;

    // match actual names as failsafe
    App_TaskEntry *entryPtr = m.findTaskByName( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::DisplayRole ).toString());
    if ( entryPtr != NULL ) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( entryPtr->name()));
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton( QMessageBox::Yes );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setWindowIcon( QIcon( ":/icons/task_remove_22" ));
        state = msgBox.exec();

        // check options
        switch ( state ) {
        case QMessageBox::Yes:
            m.removeTask( entryPtr );
            this->listModelPtr->resetModelData();
            this->currentMatch = 0;
            break;

        case QMessageBox::No:
        default:
            return;
        }
    }
}

/*
================
doneButton->clicked
================
*/
void Gui_TaskEdit::on_doneButton_clicked() {
    if ( this->ui->taskName->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText( this->tr( "Please specify task name" ));
        msgBox.setIcon( QMessageBox::Information );
        msgBox.exec();
        return;
    }

    if ( this->state() == Add ) {
        m.addTask( new App_TaskEntry( this->ui->taskName->text(), static_cast<App_TaskEntry::Types>( this->ui->taskType->currentIndex()), this->ui->taskPoints->value(), this->ui->taskMaxMulti->value(), this->ui->taskChallenge->isChecked()));
    } else if ( this->state() == Edit ) {
        // match name to be sure
        App_TaskEntry *entryPtr = m.findTaskByName( this->ui->taskList->model()->data( this->ui->taskList->currentIndex(), Qt::DisplayRole ).toString());

        if ( entryPtr == NULL ) {
            this->toggleAddEditWidget( NoState );
            return;
        }

        // set edited data
        entryPtr->setName( this->ui->taskName->text());
        entryPtr->setPoints( this->ui->taskPoints->value());
        entryPtr->setMaxMulti( this->ui->taskMaxMulti->value());
        entryPtr->setChallenge( this->ui->taskChallenge->isChecked());
        entryPtr->setType( static_cast<App_TaskEntry::Types>( this->ui->taskType->currentIndex()));

        // update anyway
        m.updateTaskList();
    }
    this->toggleAddEditWidget( NoState );
    this->listModelPtr->resetModelData();
    this->currentMatch = 0;
}

/*
================
keyPressEvent
================
*/
void Gui_TaskEdit::keyPressEvent( QKeyEvent *ePtr ) {
    // ignore close button to search for a task
    if ( ePtr->key() == Qt::Key_Return && this->ui->findTask->hasFocus()/* && !this->ui->findTask->text().isEmpty()*/) {
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
            if ( m.taskList.at( index.row())->name()/*this->listModelPtr->data( index, Qt::DisplayRole ).toString()*/.contains( matchString, Qt::CaseInsensitive )) {
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
                if ( m.taskList.at( index.row())->name()/*this->listModelPtr->data( index, Qt::DisplayRole ).toString()*/.contains( matchString, Qt::CaseInsensitive )) {
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
void Gui_TaskEdit::changeTaskType( App_TaskEntry::Types type ) {
    switch ( type ) {
    case App_TaskEntry::Check:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setDisabled( true );
        this->ui->taskChallenge->setEnabled( true );
        break;

    case App_TaskEntry::Multi:
        this->ui->taskPoints->setEnabled( true );
        this->ui->taskMaxMulti->setEnabled( true );
        this->ui->taskChallenge->setEnabled( true );
        break;

    case App_TaskEntry::Special:
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
    App_TaskEntry::Types type = static_cast<App_TaskEntry::Types>( index );
    this->changeTaskType( type );
}
