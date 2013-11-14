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
// database.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>

/*
================
loadDatabase
================
*/
void Main::loadDatabase() {
#ifdef Q_OS_UNIX
#ifdef Q_OS_ANDROID
    this->path = QString( "/sdcard/data/org.factory12.ketoevent3/" );
#else
    this->path = QString( QDir::homePath() + "/.ketoevent/" );
#endif
#else
    this->path = QString( QDir::currentPath() + "/" );
#endif

    // make path id nonexistant
    QDir dir( this->path );
    if ( !dir.exists()) {
        dir.mkpath( this->path );
        if ( !dir.exists()) {
            m.error( StrFatalError + this->tr( "could not create database path\n" ));
            return;
        }
    }

    // create database

    this->databasePath = this->path + "ketoevent.db";
    QFile database( this->databasePath );
    QSqlDatabase db;

    // failsafe
    if ( !db.isDriverAvailable( "QSQLITE" ))
        m.error( StrFatalError + this->tr( "sqlite not present on the system\n" ));

    // set sqlite driver
    db = QSqlDatabase::addDatabase( "QSQLITE" );

    // touch file if empty
    if ( !database.exists()) {
        database.open( QFile::WriteOnly );
        database.close();
    }

    // set path and open
    db.setDatabaseName( this->databasePath );
    if ( !db.open())
        m.error( StrFatalError + this->tr( "could not load task database\n" ));

    // create query
    QSqlQuery query;

    // create initial table structure (if non-existant)
    // TODO: add compatibility layer for the 2013 event
    if ( !query.exec( "create table if not exists tasks ( id integer primary key, name varchar( 256 ) unique, points integer, multi integer, style integer, type integer, parent integer )" ) ||
         !query.exec( "create table if not exists teams ( id integer primary key, name varchar( 64 ) unique, members integer, finishTime varchar( 5 ), lock integer, evaluatorId integer )" ) ||
         !query.exec( "create table if not exists evaluators ( id integer primary key, name varchar( 64 ) unique )" ) ||
         !query.exec( "create table if not exists events ( id integer primary key, api integer, name varchar( 64 ) unique, minMembers integer, maxMembers integer, startTime varchar( 5 ), finishTime varchar( 5 ), finalTime varchar( 5 ), penalty integer, singleCombo integer, doubleCombo integer, tripleCombo integer )" ) ||
         !query.exec( "create table if not exists logs ( id integer primary key, value integer, combo integer, taskId integer, teamId integer )" )
         ) {
        m.error( StrFatalError + this->tr( "could not create internal database structure\n" ));
    }

    // delete orphaned logs on init
    this->removeOrphanedLogs();

    // load entries
    this->loadEvents();
    this->loadTasks();
    this->loadTeams();
    this->loadLogs();
}

/*
================
importDatabase (testing)
================
*/
void Main::importDatabase( const QString &path ) {
    QList<QPair<int, QString> > teamMatchList;
    QList<QPair<int, QString> > taskMatchList;

    // create query
    QSqlQuery query;

    // attach the new database
    query.exec( QString( "attach '%1' as toMerge" ).arg( path ));

    //
    // first add teams
    //
    query.exec( "select * from toMerge.teams" );

    // store entries
    while ( query.next()) {
        QString teamName = query.record().value( "name" ).toString();

        // check for duplicates
        if ( this->teamForName( teamName ) != NULL ) {
            // TODO: messagebox - replace?
            if ( QMessageBox::question( NULL, this->tr( "Replace team" ), this->tr( "Replace logs for team \"%1\"?" ).arg( teamName ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
                this->removeTeam( teamName );
        }

        // store temp value
        QList<QPair<int, QString> > teamMatch;
        teamMatch.append( qMakePair( query.record().value( "id" ).toInt(), teamName ));
        teamMatchList << teamMatch;

        // add to database
        this->addTeam( query.record().value( "name" ).toString(), query.record().value( "members" ).toInt(), QTime::fromString( query.record().value( "finishTime" ).toString(), "hh:mm" ), query.record().value( "lock" ).toBool());
    }

    //
    // then add tasks
    //
    query.exec( "select * from toMerge.tasks" );

    // store entries
    while ( query.next()) {
        // store temp value
        QList<QPair<int, QString> > taskMatch;
        taskMatch.append( qMakePair( query.record().value( "id" ).toInt(), query.record().value( "name" ).toString()));
        taskMatchList << taskMatch;

        // add to main database
        // duplicates are ignored
        this->addTask( query.record().value( "name" ).toString(), query.record().value( "points" ).toInt(), query.record().value( "multi" ).toInt(), static_cast<TaskEntry::Types>( query.record().value( "type" ).toInt()), static_cast<TaskEntry::Styles>( query.record().value( "style" ).toInt()));
    }

    //
    // next part is tricky one:
    // we have to match log by old taskId and teamId
    // I'm sure it can be done more efficiently
    //
    QPair<int, QString> teamMatchPtr;
    foreach ( teamMatchPtr, teamMatchList ) {
        TeamEntry *teamPtr;

        // first find the new team (if none, don't bother)
        teamPtr = m.teamForName( teamMatchPtr.second );
        if ( teamPtr == NULL )
            return;

        // then get all logs for the team
        query.exec( QString( "select * from toMerge.logs where teamId=%1" ).arg( teamMatchPtr.first ));

        // cycle through logs
        while ( query.next()) {
            // first find the task (it may be imported or already existing)
            QPair<int, QString> taskMatchPtr;
            foreach ( taskMatchPtr, taskMatchList ) {
                if ( taskMatchPtr.first == query.record().value( 2 ).toInt())
                    break;
            }
            TaskEntry *taskPtr = m.taskForName( taskMatchPtr.second );
            if ( taskPtr == NULL )
                return;

            // then add log
            this->addLog( taskPtr->id(), teamPtr->id(), query.record().value( "value" ).toInt(), static_cast<LogEntry::Combos>( query.record().value( "combo" ).toInt()));
        }
    }

    // detach the new database
    query.exec( "detach toMerge" );
}
