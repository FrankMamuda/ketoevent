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
    this->connect( this, SIGNAL( changed()), Main::instance(), SLOT( update()));
}

/**
 * @brief Log::points
 * @return
 */
int Log::points() const {
    Task *task;

    task = Task::forId( this->taskId());
    if ( task != nullptr )
        return task->calculate( this->id());

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
    Log *log = nullptr;
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "adding a new log - taskId - %1; teamId - %2; value - %3; comboId - %4\n" ).arg( taskId ).arg( teamId ).arg( value ).arg( comboId ), Common::LogDebug );

    // avoid duplicates
    foreach ( log, Main::instance()->logList ) {
        if ( log->taskId() == taskId && log->teamId() == teamId )
            return log;
    }

    // add new log
    query.prepare( "insert into logs values ( null, :value, :taskId, :teamId, :comboId )" );
    query.bindValue( ":value", value );
    query.bindValue( ":taskId", taskId );
    query.bindValue( ":teamId", teamId );
    query.bindValue( ":comboId", comboId );

    if ( !query.exec()) {
        Common::error( CLSoftError, QObject::tr( "could not add log, reason: %1\n" ).arg( query.lastError().text()));
        return nullptr;
    }

    // select the new entry
    query.exec( QString( "select * from logs where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next()) {
        log = new Log( query.record(), "logs" );
        Main::instance()->logList << log;

        // add to list if none (for imports)
        Team *team = Team::forId( teamId );
        if ( team != nullptr ) {
            // FIXME: why here and in taskwidget.cpp?
            if ( !team->logList.contains( log ))
                team->logList << log;
        }
    }
    return log;
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
        Log *log = new Log( query.record(), "logs" );
        Team *team = Team::forId( log->teamId());
        if ( team == nullptr )
            return;

        team->logList << log;
        Main::instance()->logList << log;
    }
}

/**
 * @brief Log::forId
 * @param id
 * @return
 */
Log *Log::forId( int id ) {
    foreach ( Log *log, Main::instance()->logList ) {
        if ( log->id() == id )
            return log;
    }
    return nullptr;
}

/**
 * @brief Log::forIds
 * @param teamId
 * @param taskId
 * @return
 */
Log *Log::forIds( int teamId, int taskId ) {
    Team *team = Team::forId( teamId );
    if ( team == nullptr )
        return nullptr;

    foreach ( Log *log, team->logList ) {
        if ( log->taskId() == taskId && log->teamId() == teamId )
            return log;
    }
    return nullptr;
}
