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
// log.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
addLog
================
*/
LogEntry *Main::addLog( int taskId, int teamId, int value, LogEntry::Combos combo ) {
    LogEntry *logPtr = NULL;
    QSqlQuery query;

    // avoid duplicates
    foreach ( logPtr, this->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }

    // add new log
    if ( !query.exec( QString( "insert into logs values ( null, %1, %2, %3, %4 )" )
                      .arg( value )
                      .arg( static_cast<int>( combo ))
                      .arg( taskId )
                      .arg( teamId )
                      )) {
        this->error( StrSoftError + QString( "could not add log, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from logs where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next()) {
        logPtr = new LogEntry( query.record(), "logs" );
        this->logList << logPtr;
    }
    return logPtr;
}


/*
================
loadLogs
================
*/
void Main::loadLogs() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from logs" );

    // store entries
    while ( query.next()) {
        LogEntry *logPtr = new LogEntry( query.record(), "logs" );
        this->teamForId( logPtr->teamId())->logList << logPtr;
        this->logList << logPtr;
    }
}

/*
================
logForId
================
*/
LogEntry *Main::logForId( int id ) {
    foreach ( LogEntry *logPtr, this->logList ) {
        if ( logPtr->id() == id )
            return logPtr;
    }
    return NULL;
}

/*
================
logForIds
================
*/
LogEntry *Main::logForIds( int teamId, int taskId ) {
    TeamEntry *teamPtr = this->teamForId( teamId );
    if ( teamPtr == NULL )
        return NULL;

    foreach ( LogEntry *logPtr, teamPtr->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }
    return NULL;
}

/*
================
removeOrphanedLogs
================
*/
void Main::removeOrphanedLogs() {
    // create query
    QSqlQuery query;

    // remove orphaned logs (fixes crash with invalid teamId/taskId)
    if ( !query.exec( "delete from logs where value=0" ) || !query.exec( "delete from logs where teamId not in ( select id from teams )" ) || !query.exec( "delete from logs where taskId not in ( select id from tasks )" ))
        m.error( StrSoftError + QString( "could not delete orphaned logs, reason: %1\n" ).arg( query.lastError().text()));
}