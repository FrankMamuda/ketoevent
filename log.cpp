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
// log.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Main::addLog
 * @param taskId
 * @param teamId
 * @param value
 * @param comboId
 * @return
 */
Log *Main::addLog( int taskId, int teamId, int value, int comboId ) {
    Log *logPtr = NULL;
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "adding a new log - taskId - %1; teamId - %2; value - %3; comboId - %4\n" ).arg( taskId ).arg( teamId ).arg( value ).arg( comboId ), Main::LogDebug );

    // avoid duplicates
    foreach ( logPtr, this->base.logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }

    // add new log
#ifdef SQL_PREPARE_STATEMENTS
    query.prepare( "insert into logs values ( null, :value, :taskId, :teamId, :comboId )" );
    query.bindValue( ":value", value );
    query.bindValue( ":taskId", taskId );
    query.bindValue( ":teamId", teamId );
    query.bindValue( ":comboId", comboId );

    if ( !query.exec())
#else
    if ( !query.exec( QString( "insert into logs values ( null, %1, %2, %3, %4 )" )
                      .arg( value )
                      .arg( taskId )
                      .arg( teamId )
                      .arg( comboId )
                      ))
#endif
        this->error( StrSoftError, QString( "could not add log, reason: %1\n" ).arg( query.lastError().text()));

    // select the new entry
    query.exec( QString( "select * from logs where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next()) {
        logPtr = new Log( query.record(), "logs" );
        this->base.logList << logPtr;
    }
    return logPtr;
}

/**
 * @brief Main::loadLogs
 * @param import
 * @param store
 */
void Main::loadLogs( bool import, bool store ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "loading logs form database\n" ), Main::LogDebug );

    // read stuff
    if ( import )
        query.exec( "select * from merge.logs" );
    else
        query.exec( "select * from logs" );

    // store entries
    while ( query.next()) {
        Log *logPtr = new Log( query.record(), "logs" );
        Team *teamPtr = this->teamForId( logPtr->teamId(), import );
        if ( teamPtr == NULL )
            return;

        teamPtr->logList << logPtr;

        if ( import ) {
            logPtr->setImported();
            this->import.logList << logPtr;

            if ( store )
                logPtr->store();
        } else
            this->base.logList << logPtr;
    }
}

/**
 * @brief Main::logForId
 * @param id
 * @return
 */
Log *Main::logForId( int id ) {
    foreach ( Log *logPtr, this->base.logList ) {
        if ( logPtr->id() == id )
            return logPtr;
    }
    return NULL;
}

/**
 * @brief Main::logForIds
 * @param teamId
 * @param taskId
 * @return
 */
Log *Main::logForIds( int teamId, int taskId ) {
    Team *teamPtr = this->teamForId( teamId );
    if ( teamPtr == NULL )
        return NULL;

    foreach ( Log *logPtr, teamPtr->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }
    return NULL;
}

/**
 * @brief Main::removeOrphanedLogs
 */
void Main::removeOrphanedLogs() {
    // create query
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "removing orphaned logs\n" ), Main::LogDebug );

    // remove orphaned logs (fixes crash with invalid teamId/taskId)
    if ( !query.exec( "delete from logs where value=0" ) || !query.exec( "delete from logs where teamId not in ( select id from teams )" ) || !query.exec( "delete from logs where taskId not in ( select id from tasks )" ))
        this->error( StrSoftError, QString( "could not delete orphaned logs, reason: %1\n" ).arg( query.lastError().text()));
}
