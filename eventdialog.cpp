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
#include "eventdialog.h"
#include "mainwindow.h"
#include "event.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileDialog>
#include "QSqlError"
#include <QInputDialog>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>

/**
 * @brief EventDialog::EventDialog
 * @param parent
 */
EventDialog::EventDialog( QWidget *parent ) : Dialog( parent ), ui( new Ui::EventDialog ) {
    // set up gui
    this->ui->setupUi( this );

    // bind settings vars
    if ( Main::instance()->isInitialised())
        this->bindVars();

    this->fillEvents();
}

/**
 * @brief EventDialog::fillEvents
 */
void EventDialog::fillEvents() {
    int y = 0, id = -1;
    Event *event;

    if ( this->variablesLocked())
        return;

    // clear event list
    this->ui->eventCombo->clear();

    // set last event
    event = Event::forId( Variable::instance()->integer( "currentEvent" ));
    if ( event != nullptr )
        id = event->id();

    // fill the combobox with events
    foreach ( Event *ePtr, Main::instance()->eventList ) {
        this->ui->eventCombo->addItem( ePtr->name(), ePtr->id());
        if ( ePtr->id() == id )
            this->ui->eventCombo->setCurrentIndex( y );
        y++;
    }
}

/**
 * @brief EventDialog::~EventDialog
 */
EventDialog::~EventDialog() {
    this->unbindVars();
    delete ui;
}

/**
 * @brief EventDialog::bindVars
 */
void EventDialog::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    this->bindVariable( "startTime", this->ui->startTime );
    this->bindVariable( "finishTime", this->ui->finishTime );
    this->bindVariable( "finalTime", this->ui->finalTime );
    this->bindVariable( "penalty", this->ui->penalty );
    this->bindVariable( "comboOfTwo", this->ui->sCombo );
    this->bindVariable( "comboOfThree", this->ui->dCombo );
    this->bindVariable( "comboOfFourAndMore", this->ui->tCombo );
    this->bindVariable( "minMembers", this->ui->min );
    this->bindVariable( "maxMembers", this->ui->max );

    // refocus
    this->ui->buttonClose->setFocus();

    // unlock vars
    this->lockVariables( false );
}

/**
 * @brief EventDialog::on_buttonClose_clicked
 */
void EventDialog::on_buttonClose_clicked() {
    this->validate();
    this->onAccepted();
}

/**
 * @brief EventDialog::on_eventCombo_currentIndexChanged
 * @param index
 */
void EventDialog::on_eventCombo_currentIndexChanged( int index ) {
    if ( index == -1 )
        return;

    // check for errors just in case
    this->validate();

    // set current event
    Event *event = Event::forId( this->ui->eventCombo->itemData( index ).toInt());
    if ( event != nullptr && event != Event::active()) {
        Event::setActive( event );
        this->lockVariables();
        this->updateVars();
        this->lockVariables( false );
    }
}

/**
 * @brief EventDialog::validate
 */
void EventDialog::validate() {
    // start time must be earlier than finish/final times
    if ( this->ui->startTime->time() > this->ui->finishTime->time() || this->ui->startTime->time() > this->ui->finalTime->time())
        this->ui->startTime->setTime( this->ui->finishTime->time().addSecs( -60 ));

    // finish time might match final time, but not a minute more
    if ( this->ui->finishTime->time() > this->ui->finalTime->time())
        this->ui->finishTime->setTime( this->ui->finalTime->time());

    // minMembers <= maxMembers!
    if ( this->ui->min->value() > this->ui->max->value())
        this->ui->min->setValue( this->ui->max->value());
}

/**
 * @brief EventDialog::on_actionAddEvent_triggered
 */
void EventDialog::on_actionAddEvent_triggered() {
    bool ok;
    QString title;

    title = QInputDialog::getText( this, this->tr( "Add an event" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok );
    if ( ok && !title.isEmpty()) {
        Event::add( title );
        this->fillEvents();
    } else {
        QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
    }
}

/**
 * @brief EventDialog::on_actionRemoveEvent_triggered
 */
void EventDialog::on_actionRemoveEvent_triggered() {
    QMessageBox msgBox;
    int state;
    Event *event = Event::active();
    QSqlQuery query;

    // make sure we cannot delete all events
    if ( Main::instance()->eventList.count() <= 1 ) {
        QMessageBox::warning( this, "Error", "Cannot remove last remaining event", QMessageBox::Ok );
        return;
    }

    // allow to reconsider
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( Event::active()->name()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setWindowIcon( QIcon( ":/icons/event_remove.png" ));
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        // remove from memory
        Main::instance()->eventList.removeOne( Event::active());
        Event::setActive( Main::instance()->eventList.first());

        // remove from database
        query.exec( QString( "delete from events where id=%1" ).arg( event->id()));
        delete event;

        // reset
        this->fillEvents();
        break;

    case QMessageBox::No:
    default:
        return;
    }
}

/**
 * @brief EventDialog::on_actionImportLogs_triggered
 */
void EventDialog::on_actionImportLogs_triggered() {
    QString path, filePath;
    QStringList filePaths;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePaths = QFileDialog::getOpenFileNames( this, this->tr( "Select database" ), path, this->tr( "Database (*.db)" ));

    // go through all list
    foreach ( filePath, filePaths ) {
        // check for empty filenames
        if ( filePath.isEmpty())
            return;

        // check if path is valid
        if ( !QFileInfo( filePath ).absoluteDir().isReadable())
            return;

        // importing database
        if ( filePath.endsWith( ".db" )) {

            // avoid importing the same database
            if ( !QString::compare( filePath, Main::instance()->path )) {
                Common::error( StrSoftError, "cannot import current database\n" );
                return;
            }

            // import database
            Database::attach( filePath, Database::LogImport );
        }
    }

    // close window
    this->onAccepted();
}

/**
 * @brief EventDialog::on_actionImportTasks_triggered
 */
void EventDialog::on_actionImportTasks_triggered() {
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getOpenFileName( this, this->tr( "Select database or csv list" ), path, this->tr( "Database, CSV list or 2012 XML (*.db *.csv)" ));

    // check for empty filenames
    if ( filePath.isEmpty())
        return;

    // check if path is valid
    if ( !QFileInfo( filePath ).absoluteDir().isReadable())
        return;

    // importing database
    if ( filePath.endsWith( ".db" )) {
        // avoid importing the same database
        if ( !QString::compare( filePath, Main::instance()->path )) {
            Common::error( StrSoftError, "cannot import current database\n" );
            return;
        }

        // import database
        Database::attach( filePath, Database::TaskImport );
    }
    // importing csv
    else if ( filePath.endsWith( ".csv" )) {
        QStringList tasks, info;
        QFile csvList( filePath );

        csvList.open( QFile::ReadOnly );
        QTextStream in( &csvList );

#ifdef Q_OS_WIN
        in.setCodec( "Windows-1257" );
#else
        in.setCodec( "UTF-8" );
#endif
        tasks = QString( in.readAll().constData()).split( "\n" );

        // throw out header
        tasks.takeFirst();

        foreach ( QString task, tasks ) {
            info = task.split( ";" );

            if ( info.count() == 6 ) {
                Task *task = Task::forName( info.at( 0 ));

                if ( task == nullptr ) {
                    Task::add( info.at( 0 ),                // name
                               info.at( 2 ).toInt(),        // points
                               info.at( 3 ).toInt(),        // multi
                               static_cast<Task::Types>
                               ( info.at( 5 ).toInt()),     // type
                               static_cast<Task::Styles>
                               ( info.at( 4 ).toInt()),     // style
                               info.at( 1 ));               // description
                } else {
#ifdef APPLET_DEBUG
                    Common::print( StrMsg + this->tr( "updating task \"%1\"\n" ).arg( info.at( 0 )).arg( info.count()), Common::EventDebug );
#endif
                    task->setPoints( info.at( 2 ).toInt());
                    task->setMulti( info.at( 3 ).toInt());
                    task->setType( static_cast<Task::Types>( info.at( 5 ).toInt()));
                    task->setStyle( static_cast<Task::Styles>( info.at( 4 ).toInt()));
                    task->setDescription( info.at( 1 ));
                }
            }
        }

        csvList.close();

        MainWindow *mainWindow = qobject_cast<MainWindow*>( this->parent());
        if ( mainWindow != nullptr )
            mainWindow->fillTasks();
    } else {
        Common::error( StrSoftError, this->tr( "unknown task storage format\n" ));
        return;
    }

    // close window
    this->onAccepted();
}

/**
 * @brief EventDialog::on_actionExportEvent_triggered
 */
void EventDialog::on_actionExportEvent_triggered() {
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
    if ( !QString::compare( path, Main::instance()->path )) {
        Common::error( StrSoftError, this->tr( "cannot overwrite current database\n" ));
        return;
    }

    // touch file
    if ( database.open( QFile::WriteOnly | QFile::Truncate ))
        database.close();

    // check if exists
    if ( !database.exists()) {
        Common::error( StrSoftError, this->tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        return;
    }

    // attach the new database
    if ( !query.exec( QString( "attach '%1' as export" ).arg( path ))) {
        Common::error( StrSoftError, this->tr( "could not attach database, reason - \"%1\"\n" ).arg( query.lastError().text()));
        return;
    }

    Database::createStructure( "export." );
    query.exec( QString( "insert into export.tasks select * from tasks where eventId=%1" ).arg( Event::active()->id()));
    query.exec( QString( "insert into export.events select * from events where id=%1" ).arg( Event::active()->id()));
    query.exec( "detach export" );
}

/**
 * @brief EventDialog::on_actionExportTasks_triggered
 */
void EventDialog::on_actionExportTasks_triggered() {
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
        foreach ( Task *task, Event::active()->taskList ) {
            out << QString( "%1;%2;%3;%4;%5;%6%7" )
                   .arg( task->name().replace( ';', ',' ))
                   .arg( task->description().replace( ';', ',' ))
                   .arg( task->points())
                   .arg( task->multi())
                   .arg( task->style())
                   .arg( task->type())
       #ifdef Q_OS_WIN
                   .arg( "\r\n" );
#else
                   .arg( "\n" );
#endif
        }
    }
    taskList.close();
}

/**
 * @brief EventDialog::on_actionRename_triggered
 */
void EventDialog::on_actionRename_triggered() {
    bool ok;
    QString title;

    title = QInputDialog::getText( this, this->tr( "Add an event" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok );
    if ( ok && !title.isEmpty()) {
        // FIXME: check for duplicates
        Event::active()->setName( title );
        this->fillEvents();
    } else {
        QMessageBox::warning( this, this->tr( "Set event title" ), this->tr( "Event title not specified" ));
    }
}
