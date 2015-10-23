/*
===========================================================================
Copyright (C) 2013-2015 Avotu Briezhaudzetava

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
#include "gui_event.h"
#include "ui_gui_event.h"
#include "gui_main.h"
#include "evententry.h"
#include "gui_addedit.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileDialog>
#include "QSqlError"
#include <QStringList>

// FIXME: event doesnt change on MAINWINDOW

/*
================
construct
================
*/
Gui_Event::Gui_Event( QWidget *parent ) : Gui_Dialog( parent ), ui( new Ui::Gui_Event ) {
    // set up gui
    ui->setupUi( this );

    // bind settings vars
    if ( m.isInitialized())
        this->bindVars();

    this->fillEvents();
    this->setImported( false );
}

/*
================
fillEvents
================
*/
void Gui_Event::fillEvents() {
    int y = 0, id = -1;
    EventEntry *eventPtr;

    if ( this->variablesLocked())
        return;

    // clear event list
    this->ui->eventCombo->clear();

    // set last event
    eventPtr = m.eventForId( m.cvar( "currentEvent" )->integer());
    if ( eventPtr != NULL )
        id = eventPtr->id();

    // fill the combobox with events
    foreach ( EventEntry *ePtr, m.base.eventList ) {
        this->ui->eventCombo->addItem( ePtr->name(), ePtr->id());
        if ( ePtr->id() == id )
            this->ui->eventCombo->setCurrentIndex( y );
        y++;
    }
}

/*
================
destruct
================
*/
Gui_Event::~Gui_Event() {
    //this->disconnect( this->ui->titleEdit, SIGNAL( textChanged( QString )));
    this->unbindVars();
    delete ui;
}

/*
================
bindVars
================
*/
void Gui_Event::bindVars() {
    //Gui_Main *gui;

    // lock vars
    this->lockVariables();

    // bind vars
    //this->bindVariable( "key", object );
    this->bindVariable( "startTime", this->ui->startTime );
    this->bindVariable( "finishTime", this->ui->finishTime );
    this->bindVariable( "finalTime", this->ui->finalTime );
    this->bindVariable( "penalty", this->ui->penalty );
    this->bindVariable( "comboOfTwo", this->ui->sCombo );
    this->bindVariable( "comboOfThree", this->ui->dCombo );
    this->bindVariable( "comboOfFourAndMore", this->ui->tCombo );
    this->bindVariable( "minMembers", this->ui->min );
    this->bindVariable( "maxMembers", this->ui->max );
    //this->bindVariable( "name", this->ui->titleEdit );

    // connect for updates
    //gui = qobject_cast<Gui_Main*>( this->parent());
    //if ( gui != NULL )
    //    this->connect( this->ui->titleEdit, SIGNAL( textChanged( QString )), this, SLOT( fillEvents()));

    this->ui->buttonClose->setFocus();

    // unlock vars
    this->lockVariables( false );
}

/*
================
buttonClose->clicked
================
*/
void Gui_Event::on_buttonClose_clicked() {
    this->validate();
    this->onAccepted();
}

/*
================
eventCombo->currentIndexChanged
================
*/
void Gui_Event::on_eventCombo_currentIndexChanged( int index ) {
    if ( index == -1 )
        return;

    // check for errors just in case
    this->validate();

    // set current event
    EventEntry *eventPtr = m.eventForId( this->ui->eventCombo->itemData( index ).toInt());
    if ( eventPtr != NULL && eventPtr != m.currentEvent()) {
        m.setCurrentEvent( eventPtr );
        this->lockVariables();
        this->updateVars();
        this->lockVariables( false );
    }
}

/*
================
validate
================
*/
void Gui_Event::validate() {
    // start time must be earlier than finish/final times
    if ( this->ui->startTime->time() > this->ui->finishTime->time() || this->ui->startTime->time() > this->ui->finalTime->time())
        this->ui->startTime->setTime( this->ui->finishTime->time().addSecs( -60 ));

    // finish time might match final time, but not a minute more
    if ( this->ui->finishTime->time() > this->ui->finalTime->time())
        this->ui->finishTime->setTime( this->ui->finalTime->time());

    // minMembers <= maxMembers!
    if ( this->ui->min->value() > this->ui->max->value())
        this->ui->min->setValue( this->ui->max->value());

    // as for combos - do as you please
}

/*
================
actionAddEvent->triggered
================
*/
void Gui_Event::on_actionAddEvent_triggered() {
    Gui_AddEdit evAdd( Gui_AddEdit::EventDialog, Gui_AddEdit::Add, -1, this );
    evAdd.exec();
}

/*
================
actionRemoveEvent->triggered
================
*/
void Gui_Event::on_actionRemoveEvent_triggered() {
    QMessageBox msgBox;
    int state;
    EventEntry *eventPtr = m.currentEvent();
    QSqlQuery query;

    // make sure we cannot delete all events
    if ( m.base.eventList.count() <= 1 ) {
        QMessageBox::warning( this, "Error", "Cannot remove last remaining event", QMessageBox::Ok );
        return;
    }

    // allow to reconsider
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( m.currentEvent()->name()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setWindowIcon( QIcon( ":/icons/event_remove.png" ));
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        // remove from memory
        m.base.eventList.removeOne( m.currentEvent());
        m.setCurrentEvent( m.base.eventList.first());

        // remove from database
        query.exec( QString( "delete from events where id=%1" ).arg( eventPtr->id()));
        delete eventPtr;

        // reset
        this->fillEvents();
        break;

    case QMessageBox::No:
    default:
        return;
    }
}

/*
================
actionImportLogs->triggered
================
*/
void Gui_Event::on_actionImportLogs_triggered() {
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getOpenFileName( this, this->tr( "Select database" ), path, this->tr( "Database (*.db)" ));

    // check for empty filenames
    if ( filePath.isEmpty())
        return;

    // check if path is valid
    if ( !QFileInfo( filePath ).absoluteDir().isReadable())
        return;

    // avoid importing the same database
    if ( !QString::compare( filePath, m.path )) {
        m.error( StrSoftError, "cannot import current database\n" );
        return;
    }

    // import database
    m.attachDatabase( filePath, Main::LogImport );

    // mark as imported
    this->setImported();

    // close window
    this->onAccepted();
}

/*
================
actionImportTasks->triggered
================
*/
void Gui_Event::on_actionImportTasks_triggered() {
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getOpenFileName( this, this->tr( "Select database or csv list" ), path, this->tr( "Database or CSV list (*.db *.csv)" ));

    // check for empty filenames
    if ( filePath.isEmpty())
        return;

    // check if path is valid
    if ( !QFileInfo( filePath ).absoluteDir().isReadable())
        return;

    // importing database
    if ( filePath.endsWith( ".db" )) {
        // avoid importing the same database
        if ( !QString::compare( filePath, m.path )) {
            m.error( StrSoftError, "cannot import current database\n" );
            return;
        }

        // import database
        m.attachDatabase( filePath, Main::TaskImport );
    }
    // importing csv
    else if ( filePath.endsWith( ".csv" )) {
        QStringList tasks, info;
        QFile csvList( filePath );
        csvList.open( QFile::ReadOnly );
        tasks = QString( csvList.readAll().constData()).split( "\n" );

        // throw out header
        tasks.takeFirst();

        foreach ( QString task, tasks ) {
            info = task.split( ";" );

            if ( info.count() == 6 ) {
                TaskEntry *taskPtr = m.taskForName( info.at( 0 ));

                if ( taskPtr == NULL ) {
                    m.print( QString( "Adding new task \"%1\"\n" ).arg( info.at( 0 )).arg( info.count()), Main::System );
                    m.addTask( info.at( 0 ),                // name
                               info.at( 2 ).toInt(),        // points
                               info.at( 3 ).toInt(),        // multi
                               static_cast<TaskEntry::Types>
                               ( info.at( 5 ).toInt()),     // type
                               static_cast<TaskEntry::Styles>
                               ( info.at( 4 ).toInt()),     // style
                               info.at( 1 ));               // description
                } else {
                    m.print( QString( "Updating task \"%1\"\n" ).arg( info.at( 0 )).arg( info.count()), Main::System );
                    taskPtr->setPoints( info.at( 2 ).toInt());
                    taskPtr->setMulti( info.at( 3 ).toInt());
                    taskPtr->setType( static_cast<TaskEntry::Types>( info.at( 5 ).toInt()));
                    taskPtr->setStyle( static_cast<TaskEntry::Styles>( info.at( 4 ).toInt()));
                    taskPtr->setDescription( info.at( 1 ));
                }
            }
        }

        csvList.close();
    } else {
        m.error( StrSoftError, "unknown task storage format\n" );
        return;
    }

    // mark as imported
    this->setImported();

    // close window
    this->onAccepted();
}

/*
================
actionExportEvent->triggered
================
*/
void Gui_Event::on_actionExportEvent_triggered() {
    QString path;
    path = QFileDialog::getSaveFileName( this, this->tr( "Export event structure" ), QDir::homePath(), this->tr( "Database (*.db)" ));
    QSqlQuery query;

    // check for empty filenames
    if ( path.isEmpty())
        return;

    // add extension
    if ( !path.endsWith( ".db" ))
        path.append( ".db" );

    // create file
    QFile database( path );
    QFileInfo dbInfo( database );

    // forbid overwrite of the current database
    if ( !QString::compare( path, m.path )) {
        m.error( StrSoftError, "cannot overwrite current database\n" );
        return;
    }

    // touch file
    if ( database.open( QFile::WriteOnly | QFile::Truncate ))
        database.close();

    // check if exists
    if ( !database.exists()) {
        m.error( StrSoftError, this->tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        return;
    }

    // attach the new database
    if ( !query.exec( QString( "attach '%1' as export" ).arg( path ))) {
        m.error( StrSoftError, this->tr( "could not attach database, reason - \"%1\"\n" ).arg( query.lastError().text()));
        return;
    }

    m.touchDatabase( "export." );
    query.exec( QString( "insert into export.tasks select * from tasks where eventId=%1" ).arg( m.currentEvent()->id()));
    query.exec( QString( "insert into export.events select * from events where id=%1" ).arg( m.currentEvent()->id()));
    query.exec( "detach export" );
}

/*
================
actionExportTasks->triggered
================
*/
void Gui_Event::on_actionExportTasks_triggered() {
    QString path;
    path = QFileDialog::getSaveFileName( this, this->tr( "Export event structure" ), QDir::homePath(), this->tr( "CSV file (*.csv)" ));

    // check for empty filenames
    if ( path.isEmpty())
        return;

    // add extension
    if ( !path.endsWith( ".csv" ))
        path.append( ".csv" );

    // create file
    QFile taskList( path );

    // touch file
    if ( taskList.open( QFile::WriteOnly | QFile::Truncate )) {
        QTextStream out( &taskList );
#ifdef Q_OS_WIN
        out.setCodec( "Windows-1257" );
#else
        out.setCodec( "UTF-8" );
#endif
        out << this->tr( "Task name;Description;Points;Multi;Style;Type" )
       #ifdef Q_OS_WIN
               .append( "\r" )
       #endif
               .append( "\n" );
        foreach ( TaskEntry *taskPtr, m.currentEvent()->taskList ) {
            out << QString( "%1;%2;%3;%4;%5;%6%7" )
                   .arg( taskPtr->name().replace( ';', ',' ))
                   .arg( taskPtr->description().replace( ';', ',' ))
                   .arg( taskPtr->points())
                   .arg( taskPtr->multi())
                   .arg( taskPtr->style())
                   .arg( taskPtr->type())
       #ifdef Q_OS_WIN
                   .arg( "\r\n" );
#else
                   .arg( "\n" );
#endif
        }
    }
    taskList.close();
}

/*
================
actionRename->triggered
================
*/
void Gui_Event::on_actionRename_triggered() {
    Gui_AddEdit evAdd( Gui_AddEdit::EventDialog, Gui_AddEdit::Rename, -1, this );
    evAdd.exec();
}
