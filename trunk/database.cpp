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
// database.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QApplication>

/*
================
makePath
================
*/
void Main::makePath( const QString &path ) {
    QString fullPath;

    // default path?
    if ( path.isEmpty()) {
#ifdef Q_OS_UNIX
#ifdef Q_OS_ANDROID
        fullPath = QString( "/sdcard/data/org.factory12.ketoevent3/" );
#else
        fullPath = QString( QDir::homePath() + "/.ketoevent/" );
#endif
#else
        fullPath = QString( QDir::currentPath() + "/" );
#endif
        fullPath.append( "/" );
        fullPath.append( Common::defaultDatabase );
    } else
        fullPath = path;

    // make path id nonexistant
    QFileInfo db( fullPath );

    if ( !db.absoluteDir().exists()) {
        db.absoluteDir().mkpath( fullPath );
        if ( !db.absoluteDir().exists())
            this->error( StrFatalError + this->tr( "could not create database path - \"%1\"\n" ).arg( fullPath ));
    }

    // store path
    this->path = fullPath;
}

/*
================
highestId
================
*/
int Main::highestId( IdTypes type ) const {
    int id = 1;

    switch ( type ) {
    case ComboId:
        foreach ( LogEntry *logPtr, m.base.logList ) {
            if ( logPtr->comboId() > id )
                id = logPtr->id();
        }
        break;

    case TeamId:
        foreach ( TeamEntry *teamPtr, m.base.teamList ) {
            if ( teamPtr->id() > id )
                id = teamPtr->id();
        }
        break;

    case LogId:
        foreach ( LogEntry *logPtr, m.base.logList ) {
            if ( logPtr->id() > id )
                id = logPtr->id();
        }
        break;

    case ReviewerId:
        foreach ( ReviewerEntry *reviewerPtr, m.base.reviewerList ) {
            if ( reviewerPtr->id() > id )
                id = reviewerPtr->id();
        }
        break;

    case NoId:
    default:
        id = 0;
        break;
    }
    return id;
}

/*
================
encrypt
================
*/
static QString encrypt( const QString &input ) {
    QCryptographicHash *hash = new QCryptographicHash( QCryptographicHash::Md5 );
    hash->addData( input.toLatin1().constData(), input.length());
    return QString( hash->result().toHex().constData());
}

/*
================
taskListHash
================
*/
static QString taskListHash( bool import ) {
    QList<TaskEntry*> list;
    QString taskString;
    int eventId = m.currentEvent()->id();

    if ( import )
        list = m.import.taskList;
    else
        list = m.base.taskList;

    // build unique taskList
    foreach ( TaskEntry *taskPtr, list ) {
        if ( taskPtr->eventId() != eventId )
            continue;

        taskString.append( QString( "%1%2%3%4" ).arg( taskPtr->name()).arg( taskPtr->points()).arg( taskPtr->multi()).arg( taskPtr->type()));
    }

    // generate taskList checksum (to avoid mismatches)
    return encrypt( taskString );
}

/*
================
attachDatabase
================
*/
void Main::attachDatabase( const QString &path ) {
    QSqlQuery query;
    QString dbPath = path + "import";

    // check database
    QFile::copy( path, dbPath );
    QFile database( dbPath );
    QFileInfo dbInfo( database );
    if ( !database.exists()) {
        this->error( StrSoftError + this->tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        return;
    }

    // attach the new database
    query.exec( QString( "attach '%1' as merge" ).arg( dbPath ));
    query.exec( QString( "update merge.teams set id=id+%1" ).arg( this->highestId( TeamId )));
    query.exec( QString( "update merge.logs set id=id+%1" ).arg( this->highestId( LogId )));

    // load eventList into temporary storage
    this->loadEvents( true );

    // find event by name
    int eventId = -1;
    foreach ( EventEntry *eventPtr, m.import.eventList ) {
        if ( !QString::compare( eventPtr->name(), m.currentEvent()->name())) {
            eventId = eventPtr->id();
            break;
        }
    }

    // failsafe
    if ( eventId == -1 ) {
        this->error( StrSoftError + this->tr( "database \"%1\" does not contain event \"%2\"\n" ).arg( dbInfo.fileName()).arg( m.currentEvent()->name()));
        return;
    }

    // get rid of junk
    query.exec( QString( "delete from merge.events where id!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.teams where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.logs where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.tasks where eventId!=%1" ).arg( eventId ));

    // update connections
    query.exec( QString( "update merge.teams set eventId=%1" ).arg( m.currentEvent()->id()));
    query.exec( QString( "update merge.tasks set eventId=%1" ).arg( m.currentEvent()->id()));
    query.exec( QString( "update merge.logs set teamId=teamId+%1" ).arg( this->highestId( TeamId )));
    query.exec( QString( "update merge.logs set comboId=comboId+%1" ).arg(  this->highestId( LogId )));

    // load taskList into temporary storage
    this->loadTasks( true );

    // compare task hashes
    if ( QString::compare( taskListHash( true ), taskListHash( false ))) {
        this->error( StrSoftError + this->tr( "task list mismatch\n" ));
        return;
    }

    // load teamlist into temporary storage
    this->loadTeams( true );

    // load logs into temporary storage
    this->loadLogs( true );

    // clean up
    this->import.teamList.clear();
    this->import.logList.clear();
    this->import.taskList.clear();
    this->import.reviewerList.clear();
    this->import.eventList.clear();
    query.exec( "detach merge" );
    database.remove();
}

/*
================
loadDatabase
================
*/
void Main::loadDatabase() {
    // create database
    QFile database( this->path );
    QFileInfo dbInfo( database );
    QSqlDatabase db = QSqlDatabase::database();

    // failsafe
    if ( !db.isDriverAvailable( "QSQLITE" ))
        this->error( StrFatalError + this->tr( "sqlite not present on the system\n" ));

    // set sqlite driver
    db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setHostName( "localhost" );
    db.setDatabaseName( this->path );

    // touch file if empty
    if ( !database.exists()) {
        database.open( QFile::WriteOnly );
        database.close();
        this->print( StrMsg + this->tr( "creating non-existant database - \"%1\"\n" ).arg( dbInfo.fileName()));
    }

    // set path and open
    if ( !db.open())
        this->error( StrFatalError + this->tr( "could not load task database - \"%1\"\n" ).arg( dbInfo.fileName()));

    // create query
    QSqlQuery query;

    // create initial table structure (if non-existant)
    // TODO: add compatibility layer for the 2013 event (or just stats)
    if ( !query.exec( "create table if not exists tasks ( id integer primary key, name varchar( 256 ) unique, points integer, multi integer, style integer, type integer, parent integer, eventId integer )" ) ||
         !query.exec( "create table if not exists teams ( id integer primary key, name varchar( 64 ) unique, members integer, finishTime varchar( 5 ), lock integer, reviewerId integer, eventId integer )" ) ||
         !query.exec( "create table if not exists reviewers ( id integer primary key, name varchar( 64 ) unique )" ) ||
         !query.exec( "create table if not exists events ( id integer primary key, api integer, name varchar( 64 ) unique, minMembers integer, maxMembers integer, startTime varchar( 5 ), finishTime varchar( 5 ), finalTime varchar( 5 ), penalty integer, comboOfTwo integer, comboOfThree integer, comboOfFourAndMore integer, lock integer )" ) ||
         !query.exec( "create table if not exists logs ( id integer primary key, value integer, taskId integer, teamId integer, comboId integer )" )) {
        this->error( StrFatalError + this->tr( "could not create internal database structure, reason - \"%1\"\n" ).arg( query.lastError().text()));
    }

    // delete orphaned logs on init
    this->removeOrphanedLogs();

    // load entries
    this->loadEvents();
    this->loadReviewers();
    this->loadTasks();
    this->loadTeams();
    this->loadLogs();
    this->buildEventTTList();
}

/*
================
unloadDatabase
================
*/
void Main::unloadDatabase() {
    QString connectionName;
    bool open = false;

    // close database if open and delete orphaned logs on shutdown
    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase db = QSqlDatabase::database();
        if ( db.isOpen()) {
            open = true;
            this->removeOrphanedLogs();
            connectionName = db.connectionName();
            db.close();
        }
    }

    // only now we can remove the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}
