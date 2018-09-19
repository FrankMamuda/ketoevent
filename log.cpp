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
 * @brief Log::multiplier
 * @param taskId
 * @param teamId
 * @return
 */
int Log::multiplier( const Id &taskId, const Id &teamId ) const {
    QSqlQuery query;
    query.exec( QString( "select %1, %2, %3 from %4 where %5=%6 and %7=%8" )
                .arg( this->fieldName( Log::Multi ))
                .arg( this->fieldName( Log::Task ))
                .arg( this->fieldName( Log::Team ))
                .arg( this->tableName())
                .arg( this->fieldName( Log::Task ))
                .arg( static_cast<int>( taskId ))
                .arg( this->fieldName( Log::Team ))
                .arg( static_cast<int>( teamId )));

    return query.next() ? query.value( 0 ).toInt() : 0;
}

/**
 * @brief Log::comboId
 * @param taskId
 * @param teamId
 * @return
 */
Id Log::comboId( const Id &taskId, const Id &teamId ) const {
    QSqlQuery query;
    query.exec( QString( "select %1, %2, %3 from %4 where %5=%6 and %7=%8" )
                .arg( this->fieldName( Log::Combo ))
                .arg( this->fieldName( Log::Task ))
                .arg( this->fieldName( Log::Team ))
                .arg( this->tableName())
                .arg( this->fieldName( Log::Task ))
                .arg( static_cast<int>( taskId ))
                .arg( this->fieldName( Log::Team ))
                .arg( static_cast<int>( teamId )));

    return query.next() ? static_cast<Id>( query.value( 0 ).toInt()) : Id::Invalid;
}

/**
 * @brief Log::id
 * @param taskId
 * @param teamId
 * @return
 */
Id Log::id( const Id &taskId, const Id &teamId ) const {
    QSqlQuery query;
    query.exec( QString( "select %1, %2, %3 from %4 where %5=%6 and %7=%8" )
                .arg( this->fieldName( Log::ID ))
                .arg( this->fieldName( Log::Task ))
                .arg( this->fieldName( Log::Team ))
                .arg( this->tableName())
                .arg( this->fieldName( Log::Task ))
                .arg( static_cast<int>( taskId ))
                .arg( this->fieldName( Log::Team ))
                .arg( static_cast<int>( teamId )));

    return query.next() ? static_cast<Id>( query.value( 0 ).toInt()) : Id::Invalid;
}

/**
 * @brief Log::setMultiplier
 * @param taskId
 * @param teamId
 */
void Log::setMultiplier( int multi, const Id &taskId, const Id &teamId ) {
    const Id logId = this->id( taskId, teamId );

    if ( logId != Id::Invalid ) {
        const Row row = this->row( logId );
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
