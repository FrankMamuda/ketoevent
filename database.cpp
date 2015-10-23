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
#include <QSqlDatabase>
#include <QSqlQuery>

// FIXME: app keeps adding '' quotes to event name on add and rename

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
        fullPath = QString( QDir::homePath() + "/.ketoevent/" );
#else
        fullPath = QString( QDir::currentPath() + "/" );
#endif
        fullPath.append( "/" );
        fullPath.append( Common::defaultDatabase );
    } else
        fullPath = path;

    // make path id nonexistant
    QFileInfo db( fullPath );
    QDir dir;
    dir.setPath( db.absolutePath());

    if ( !dir.exists()) {
        dir.mkpath( db.absolutePath());
        if ( !dir.exists())
            this->error( StrFatalError, this->tr( "could not create database path - \"%1\"\n" ).arg( fullPath ));
    }

    // store path
    this->path = fullPath;
    this->path.replace( "//", "/" );
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

    case TaskId:
        foreach ( TaskEntry *taskPtr, m.base.taskList ) {
            if ( taskPtr->id() > id )
                id = taskPtr->id();
        }
        break;


    case NoId:
    default:
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
        list = m.currentEvent()->taskList;

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
void Main::attachDatabase( const QString &path, Import import ) {
    QSqlQuery query;
    QString dbPath = path + "import";
    bool store = true;
    int eventId = -1;

    // write backup just in case
    this->writeBackup();

    // check database
    QFile::copy( path, dbPath );
    QFile database( dbPath );
    QFileInfo dbInfo( database );
    if ( !database.exists()) {
        this->error( StrSoftError, this->tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // attach the new database
    if ( !query.exec( QString( "attach '%1' as merge" ).arg( dbPath ))) {
        this->error( StrSoftError, this->tr( "could not attach database, reason - \"%1\"\n" ).arg( query.lastError().text()));
        goto removeDB;
    }

    if ( import == LogImport ) {
        // update teams
        if ( !query.exec( QString( "update merge.teams set id=id*-1" )) || !query.exec( QString( "update merge.teams set id=(id*-1)+%1" ).arg( this->highestId( TeamId )))) {
            this->error( StrSoftError, this->tr( "could not update teams, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }

        // update logs
        if ( !query.exec( QString( "update merge.logs set id=id*-1" )) || !query.exec( QString( "update merge.logs set id=(id*-1)+%1" ).arg( this->highestId( LogId )))) {
            this->error( StrSoftError, this->tr( "could not update logs, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }
    } else if ( import == TaskImport ) {
        store = false;

        // update tasks
        if ( !query.exec( QString( "update merge.tasks set id=id*-1" )) || !query.exec( QString( "update merge.tasks set id=(id*-1)+%1" ).arg( this->highestId( TaskId )))) {
            this->error( StrSoftError, this->tr( "could not update tasks, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }
    }

    // load eventList into temporary storage
    if ( !this->loadEvents( true )) {
        this->error( StrSoftError, this->tr( "could not load database \"%1\"\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // find event by name
    foreach ( EventEntry *eventPtr, m.import.eventList ) {
        if ( !QString::compare( eventPtr->name(), m.currentEvent()->name())) {
            eventId = eventPtr->id();
            break;
        }
    }

    // failsafe
    if ( eventId == -1 ) {
        this->error( StrSoftError, this->tr( "database \"%1\" does not contain event \"%2\"\n" ).arg( dbInfo.fileName()).arg( m.currentEvent()->name()));
        goto removeDB;
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
    query.exec( QString( "update merge.logs set comboId=comboId+%1" ).arg( this->highestId( LogId )));

    // load taskList into temporary storage
    this->loadTasks( true, !store );

    // compare task hashes
    if ( import == LogImport ) {
        if ( QString::compare( taskListHash( true ), taskListHash( false ))) {
            this->error( StrSoftError, this->tr( "task list mismatch\n" ));
            goto removeDB;
        }

        // load teamlist into temporary storage
        this->loadTeams( true, store );

        // load logs into temporary storage
        this->loadLogs( true, store );
    }

    // clean up
    this->import.teamList.clear();
    this->import.logList.clear();
    this->import.taskList.clear();
    this->import.eventList.clear();

removeDB:
    query.exec( "detach merge" );
    database.remove();
}


/*
================
touchDatabase
================
*/
void Main::touchDatabase( const QString &prefix ) {
    // create query
    QSqlQuery query;

    // create initial table structure (if non-existant)
    if ( !query.exec( QString( "create table if not exists %1tasks ( id integer primary key, name varchar( 128 ), points integer, multi integer, style integer, type integer, parent integer, eventId integer, description varchar( 512 ))" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1teams ( id integer primary key, name varchar( 64 ), members integer, finishTime varchar( 5 ), lock integer, reviewer varchar( 64 ), eventId integer )" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1events ( id integer primary key, api integer, name varchar( 64 ), minMembers integer, maxMembers integer, startTime varchar( 5 ), finishTime varchar( 5 ), finalTime varchar( 5 ), penalty integer, comboOfTwo integer, comboOfThree integer, comboOfFourAndMore integer, lock integer )" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1logs ( id integer primary key, value integer, taskId integer, teamId integer, comboId integer )" ).arg( prefix ))) {
        this->error( StrFatalError, this->tr( "could not create internal database structure, reason - \"%1\"\n" ).arg( query.lastError().text()));
    }
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
        this->error( StrFatalError, this->tr( "sqlite not present on the system\n" ));

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
        this->error( StrFatalError, this->tr( "could not load task database - \"%1\"\n" ).arg( dbInfo.fileName()));

    // create database
    this->touchDatabase();

    // delete orphaned logs on init
    this->removeOrphanedLogs();

    // enable WAL
    QSqlQuery query;
    if ( !query.exec( "PRAGMA journal_mode=WAL" ))
        this->error( StrSoftError, QString( "could not enable WAL, reason: %1\n" ).arg( query.lastError().text()));

    // load entries
    this->loadEvents();
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
            connectionName = db.connectionName();;
            db.close();

        }
    }

    // only now we can remove the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}
