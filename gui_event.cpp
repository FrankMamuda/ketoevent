/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "gui_event.h"
#include "ui_gui_event.h"
#include "gui_main.h"
#include "event.h"
#include "gui_addedit.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileDialog>
#include "QSqlError"
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>

/**
 * @brief Gui_Event::Gui_Event
 * @param parent
 */
Gui_Event::Gui_Event( QWidget *parent ) : Gui_Dialog( parent ), ui( new Ui::Gui_Event ) {
    // set up gui
    ui->setupUi( this );

    // bind settings vars
    if ( m.isInitialised())
        this->bindVars();

    this->fillEvents();
    //this->setImported( false );
}

/**
 * @brief Gui_Event::fillEvents
 */
void Gui_Event::fillEvents() {
    int y = 0, id = -1;
    Event *eventPtr;

    if ( this->variablesLocked())
        return;

    // clear event list
    this->ui->eventCombo->clear();

    // set last event
    eventPtr = Event::forId( Variable::integer( "currentEvent" ));
    if ( eventPtr != NULL )
        id = eventPtr->id();

    // fill the combobox with events
    foreach ( Event *ePtr, m.eventList ) {
        this->ui->eventCombo->addItem( ePtr->name(), ePtr->id());
        if ( ePtr->id() == id )
            this->ui->eventCombo->setCurrentIndex( y );
        y++;
    }
}

/**
 * @brief Gui_Event::~Gui_Event
 */
Gui_Event::~Gui_Event() {
    //this->disconnect( this->ui->titleEdit, SIGNAL( textChanged( QString )));
    this->unbindVars();
    delete ui;
}

/**
 * @brief Gui_Event::bindVars
 */
void Gui_Event::bindVars() {
    //Gui_Main *gui;

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
 * @brief Gui_Event::on_buttonClose_clicked
 */
void Gui_Event::on_buttonClose_clicked() {
    this->validate();
    this->onAccepted();
}

/**
 * @brief Gui_Event::on_eventCombo_currentIndexChanged
 * @param index
 */
void Gui_Event::on_eventCombo_currentIndexChanged( int index ) {
    if ( index == -1 )
        return;

    // check for errors just in case
    this->validate();

    // set current event
    Event *eventPtr = Event::forId( this->ui->eventCombo->itemData( index ).toInt());
    if ( eventPtr != NULL && eventPtr != Event::active()) {
        Event::setActive( eventPtr );
        this->lockVariables();
        this->updateVars();
        this->lockVariables( false );
    }
}

/**
 * @brief Gui_Event::validate
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
}

/**
 * @brief Gui_Event::on_actionAddEvent_triggered
 */
void Gui_Event::on_actionAddEvent_triggered() {
    Gui_AddEdit evAdd( Gui_AddEdit::EventDialog, Gui_AddEdit::Add, -1, this );
    evAdd.exec();
}

/**
 * @brief Gui_Event::on_actionRemoveEvent_triggered
 */
void Gui_Event::on_actionRemoveEvent_triggered() {
    QMessageBox msgBox;
    int state;
    Event *eventPtr = Event::active();
    QSqlQuery query;

    // make sure we cannot delete all events
    if ( m.eventList.count() <= 1 ) {
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
        m.eventList.removeOne( Event::active());
        Event::setActive( m.eventList.first());

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

/**
 * @brief Gui_Event::on_actionImportLogs_triggered
 */
void Gui_Event::on_actionImportLogs_triggered() {
    QString path, filePath;
    QStringList filePaths;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePaths = QFileDialog::getOpenFileNames( this, this->tr( "Select database or XML (2012)" ), path, this->tr( "Database or 2012 XML (*.db *.xml)" ));

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
            if ( !QString::compare( filePath, m.path )) {
                Common::error( StrSoftError, "cannot import current database\n" );
                return;
            }

            // import database
            Database::attach( filePath, Database::LogImport );

            // mark as imported
            //this->setImported();
        } else if ( filePath.endsWith( ".xml" )) {
            QFile xmlList( filePath );
            xmlList.open( QFile::ReadOnly );

            QXmlStreamReader xml( &xmlList );

            Task::add( KetoEvent::comboString, 1, 999, Task::Multi, Task::Bold, KetoEvent::comboDescription );
            int comboTaskId = Task::forName( KetoEvent::comboString )->id();

            if ( comboTaskId < 1 )
                return;

            if ( xml.readNextStartElement()) {
                if ( !QString::compare( xml.name().toString(), "team", Qt::CaseInsensitive )) {
                    QString name = xml.attributes().value( "name" ).toString();
                    int hour = xml.attributes().value( "hour" ).toInt();

                    int minute = xml.attributes().value( "minute" ).toInt();
                    QString minuteString = QString( "%1" ).arg( minute );
                    if ( minuteString.length() == 1 )
                        minuteString.prepend( "0" );

                    int members = xml.attributes().value( "members" ).toInt();
                    bool lock = static_cast<bool>( xml.attributes().value( "lock" ).toInt());

                    foreach ( Team *teamPtr, Event::active()->teamList ) {
                        if ( !QString::compare( teamPtr->name(), name ))
                            return;
                    }

                    Team::add( name, members, QTime::fromString( QString( "%1:%2" ).arg( hour ).arg( minuteString ), "HH:mm" ), "imported", lock );
                    int id = Team::forName( name, true )->id();

                    if ( id < 1 )
                        return;

                    int combos = 0;
                    while ( xml.readNextStartElement()) {
                        if ( !QString::compare( xml.name().toString(), "log", Qt::CaseInsensitive )) {
                            QString hash = xml.attributes().value( "hash" ).toString();
                            int value = xml.attributes().value( "value" ).toInt();
                            int combo = xml.attributes().value( "combo" ).toInt();
                            combos += combo;

                            bool found = false;
                            foreach ( Task *taskPtr, Event::active()->taskList ) {
                                if ( !QString::compare( Database::stringToHash( taskPtr->name()), hash, Qt::CaseInsensitive )) {
                                    if ( value > 0 )
                                        Log::add( taskPtr->id(), id, value );
                                    found = true;
                                }
                            }

                            if ( !found )
                                Common::print( StrMsg + this->tr( "unknown task with hash %1\n" ).arg( hash ), Common::DatabaseDebug );

                            xml.readNext();
                        } else {
                            xml.skipCurrentElement();
                        }
                    }

                    Log::add( comboTaskId, id, combos );
                } else {
                    Common::error( StrSoftError, this->tr( "invalid XML file\n" ));
                }
            }

            // perform refresh
            Gui_Main *guiPtr;
            guiPtr = qobject_cast<Gui_Main*>( this->parent());

            if ( guiPtr != NULL ) {
                guiPtr->fillTeams();
                guiPtr->fillTasks();
            }

            xmlList.close();
        }
    }

    // close window
    this->onAccepted();
}

/**
 * @brief Gui_Event::on_actionImportTasks_triggered
 */
void Gui_Event::on_actionImportTasks_triggered() {
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getOpenFileName( this, this->tr( "Select database, csv list or XML (2012)" ), path, this->tr( "Database, CSV list or 2012 XML (*.db *.csv *.xml)" ));

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
                Task *taskPtr = Task::forName( info.at( 0 ));

                if ( taskPtr == NULL ) {
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
                    taskPtr->setPoints( info.at( 2 ).toInt());
                    taskPtr->setMulti( info.at( 3 ).toInt());
                    taskPtr->setType( static_cast<Task::Types>( info.at( 5 ).toInt()));
                    taskPtr->setStyle( static_cast<Task::Styles>( info.at( 4 ).toInt()));
                    taskPtr->setDescription( info.at( 1 ));
                }
            }
        }

        csvList.close();

        Gui_Main *gui = qobject_cast<Gui_Main*>( this->parent());
        if ( gui != NULL )
            gui->fillTasks();
    }
    // importing XML (2012 event)
    else if ( filePath.endsWith( ".xml" )) {
        QFile xmlList( filePath );
        xmlList.open( QFile::ReadOnly );

        QXmlStreamReader xml( &xmlList );

        if ( xml.readNextStartElement()) {
            if ( !QString::compare( xml.name().toString(), "tasks", Qt::CaseInsensitive )) {
                while ( xml.readNextStartElement()) {
                    if ( !QString::compare( xml.name().toString(), "task", Qt::CaseInsensitive )) {
                        QString name = xml.attributes().value( "name" ).toString();
                        int type = xml.attributes().value( "type" ).toInt();
                        int points = xml.attributes().value( "points" ).toInt();
                        bool challenge = static_cast<bool>( xml.attributes().value( "challenge" ).toInt());
                        int max;
                        Task::Types taskType;
                        Task::Styles style = Task::Regular;

#ifdef APPLET_DEBUG
                        Common::print( StrMsg + this->tr( "new task \"%1\" with hash \"%2\"\n" ).arg( name ).arg( Database::stringToHash( name )), Common::EventDebug );
#endif
                        if ( type > 0 ) {
                            taskType = Task::Multi;

                            if ( type == 2 ) {
                                max = 10;
                                points = 1;
                                style = Task::Italic;
                            } else
                                max  = xml.attributes().value( "max" ).toInt();
                        } else
                            taskType = Task::Check;

                        if ( challenge )
                            style = Task::Bold;

                        Task::add( name, points, max, taskType, style );

                        xml.readNext();
                    } else {
                        xml.skipCurrentElement();
                    }
                }
            } else {
                Common::error( StrSoftError, this->tr( "invalid XML file\n" ));
            }
        }

        // perform refresh
        Gui_Main *guiPtr;
        guiPtr = qobject_cast<Gui_Main*>( this->parent());

        if ( guiPtr != NULL )
            guiPtr->fillTasks();

        xmlList.close();
    } else {
        Common::error( StrSoftError, this->tr( "unknown task storage format\n" ));
        return;
    }

    // close window
    this->onAccepted();
}

/**
 * @brief Gui_Event::on_actionExportEvent_triggered
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
 * @brief Gui_Event::on_actionExportTasks_triggered
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
        foreach ( Task *taskPtr, Event::active()->taskList ) {
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

/**
 * @brief Gui_Event::on_actionRename_triggered
 */
void Gui_Event::on_actionRename_triggered() {
    Gui_AddEdit evAdd( Gui_AddEdit::EventDialog, Gui_AddEdit::Rename, -1, this );
    evAdd.exec();
}
