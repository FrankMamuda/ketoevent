/*
 * Copyright (C) 2018 Factory #12
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
#include <QSqlQuery>
#include "log.h"
#include "field.h"
#include "database.h"
#include "task.h"
#include "team.h"

//
// namespaces
//
using namespace LogTable;

/**
 * @brief Log::Log
 */
Log::Log() : Table( LogTable::Name ) {
    this->addField( ID,    "id",      QVariant::UInt, "integer primary key", true, true );
    this->addField( Multi, "value",   QVariant::Int,  "integer" );
    this->addField( Task,  "taskId",  QVariant::Int,  "integer" );
    this->addField( Team,  "teamId",  QVariant::Int,  "integer" );
    this->addField( Combo, "comboId", QVariant::Int,  "integer" );
}

/**
 * @brief Log::add
 * @param taskId
 * @param teamId
 * @param multiplier
 * @param comboId
 */
void Log::add( const Id &taskId, const Id &teamId, int multiplier, const Id &comboId ) {
    Table::add( QVariantList() <<
                Database_::null <<
                multiplier <<
                static_cast<int>( taskId ) <<
                static_cast<int>( teamId ) <<
                static_cast<int>( comboId ) );
}

/**
 * @brief Log::ids
 * @param taskId
 * @param teamId
 * @return
 */
QList<Id> Log::ids( const Id &taskId, const Id &teamId ) const {
    QSqlQuery query;
    QList<Id> list;
    query.exec( QString( "select id from %1 where taskId = %2 and teamId = %3" )
                .arg( this->tableName())
                .arg( static_cast<int>( taskId ))
                .arg( static_cast<int>( teamId )));

    if ( query.next())
        list << static_cast<Id>( query.value( 0 ).toInt());

    if ( list.isEmpty())
        list << Id::Invalid;

    return list;
}

/**
 * @brief Log::multiplier
 * @param taskId
 * @param teamId
 * @return
 */
int Log::multiplier( const Id &taskId, const Id &teamId ) const {
    const QList<Id> list( this->ids( taskId, teamId ));

    if ( list.count() > 1 )
         qWarning( Database_::Debug ) << this->tr( "multiple log entries for taskId=%1, teamId=%1" )
                                         .arg( static_cast<int>( taskId ))
                                         .arg( static_cast<int>( teamId ));

    if ( list.first() != Id::Invalid )
        return this->multiplier( this->row( list.first()));

    return 0;
}

/**
 * @brief Log::comboId
 * @param taskId
 * @param teamId
 * @return
 */
Id Log::comboId( const Id &taskId, const Id &teamId ) const {
    const QList<Id> list( this->ids( taskId, teamId ));

    if ( list.count() > 1 )
         qWarning( Database_::Debug ) << this->tr( "multiple log entries for taskId=%1, teamId=%1" )
                                         .arg( static_cast<int>( taskId ))
                                         .arg( static_cast<int>( teamId ));

    if ( list.first() != Id::Invalid )
        return this->comboId( this->row( list.first()));

    return Id::Invalid;
}

/**
 * @brief Log::setMultiplier
 * @param taskId
 * @param teamId
 */
void Log::setMultiplier( int multi, const Id &taskId, const Id &teamId ) {
    const QList<Id> list( this->ids( taskId, teamId ));

    if ( list.count() > 1 )
         qWarning( Database_::Debug ) << this->tr( "multiple log entries for taskId=%1, teamId=%1" )
                                         .arg( static_cast<int>( taskId ))
                                         .arg( static_cast<int>( teamId ));

    if ( list.first() != Id::Invalid ) {
        const Row row = this->row( list.first());

        if ( row == Row::Invalid )
            return;

        if ( multi <= 0 )
            Log::instance()->remove( row );
        else
            Log::instance()->setMultiplier( row, multi );

        if ( multi <= 0 )
             qDebug() << "delete log at row" << row;
        else
            qDebug() << "change log at row" << row;
    } else {
        qDebug() << "new log";
        Log::instance()->add( taskId, teamId, multi );
    }
}
