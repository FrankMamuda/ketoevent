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
#include "log.h"
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Log::Log
 * @param record
 * @param table
 */
Log::Log( const QSqlRecord &record, const QString &table ) {
    this->setRecord( record );
    this->setTable( table );

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/**
 * @brief Log::points
 * @return
 */
int Log::points() const {
    Task *taskPtr = Task::forId( this->taskId());

    if ( taskPtr != NULL )
        return taskPtr->calculate( this->id());

    return 0;
}

/**
 * @brief Log::add
 * @param taskId
 * @param teamId
 * @param value
 * @param comboId
 * @return Log entry
 */
Log *Log::add( int taskId, int teamId, int value, int comboId ) {
    Log *logPtr = NULL;
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "adding a new log - taskId - %1; teamId - %2; value - %3; comboId - %4\n" ).arg( taskId ).arg( teamId ).arg( value ).arg( comboId ), Common::LogDebug );

    // avoid duplicates
    foreach ( logPtr, m.logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }

    // add new log
    query.prepare( "insert into logs values ( null, :value, :taskId, :teamId, :comboId )" );
    query.bindValue( ":value", value );
    query.bindValue( ":taskId", taskId );
    query.bindValue( ":teamId", teamId );
    query.bindValue( ":comboId", comboId );

    if ( !query.exec()) {
        Common::error( CLSoftError, QObject::tr( "could not add log, reason: %1\n" ).arg( query.lastError().text()));
        return NULL;
    }

    // select the new entry
    query.exec( QString( "select * from logs where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next()) {
        logPtr = new Log( query.record(), "logs" );
        m.logList << logPtr;

        // add to list if none (for imports)
        Team *teamPtr = Team::forId( teamId );
        if ( teamPtr != NULL ) {
            if ( !teamPtr->logList.contains( logPtr ))
                teamPtr->logList << logPtr;
        }
    }
    return logPtr;
}

/**
 * @brief Log::loadLogs
 */
void Log::loadLogs() {
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "loading logs form database\n" ), Common::LogDebug );

    // read all log entries
    query.exec( "select * from logs" );

    // store entries
    while ( query.next()) {
        Log *logPtr = new Log( query.record(), "logs" );
        Team *teamPtr = Team::forId( logPtr->teamId());
        if ( teamPtr == NULL )
            return;

        teamPtr->logList << logPtr;
        m.logList << logPtr;
    }
}

/**
 * @brief Log::forId
 * @param id
 * @return
 */
Log *Log::forId( int id ) {
    foreach ( Log *logPtr, m.logList ) {
        if ( logPtr->id() == id )
            return logPtr;
    }
    return NULL;
}

/**
 * @brief Log::forIds
 * @param teamId
 * @param taskId
 * @return
 */
Log *Log::forIds( int teamId, int taskId ) {
    Team *teamPtr = Team::forId( teamId );
    if ( teamPtr == NULL )
        return NULL;

    foreach ( Log *logPtr, teamPtr->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }
    return NULL;
}
