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
