/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include <QSqlError>
#include <QSqlQuery>
#include "log.h"
#include "field.h"
#include "database.h"
#include "task.h"
#include "team.h"

/**
 * @brief Log::Log
 */
Log::Log() : Table( "logs" ) {
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
                .arg( this->fieldName( Log::Multi ),
                      this->fieldName( Log::Task ),
                      this->fieldName( Log::Team ),
                      this->tableName(),
                      this->fieldName( Log::Task ),
                      QString::number( static_cast<int>( taskId )),
                      this->fieldName( Log::Team ),
                      QString::number( static_cast<int>( teamId ))));

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
                .arg( this->fieldName( Log::Combo ),
                      this->fieldName( Log::Task ),
                      this->fieldName( Log::Team ),
                      this->tableName(),
                      this->fieldName( Log::Task ),
                      QString::number( static_cast<int>( taskId )),
                      this->fieldName( Log::Team ),
                      QString::number( static_cast<int>( teamId ))));

    return query.next() ? static_cast<Id>( query.value( 0 ).toInt()) : Id::Invalid;
}

/**
 * @brief Log::removeOrphanedEntries
 */
void Log::removeOrphanedEntries() {
    QSqlQuery query;

    // remove orphaned logs
    query.exec( QString( "delete from %1 where %2 not in (select %3 from %4) or %5 not in (select %6 from %7)" )
                .arg( this->tableName(),
                      this->fieldName( Team ),
                      Team::instance().fieldName( Team::ID ),
                      Team::instance().tableName(),
                      this->fieldName( Task ),
                      Task::instance().fieldName( Task::ID ),
                      Task::instance().tableName()));

    // delete duplicate logs
    for ( int y = 0; y < Team::instance().count(); y++ ) {
        const int teamId = static_cast<int>( Team::instance().id( Team::instance().row( y )));

        // find duplicate log entries:
        //   (multiple instances of same taskId & teamId)
        query.exec( QString( "SELECT %1, %2, COUNT(*) FROM %3 WHERE %2=%4 GROUP BY %1, %2 HAVING COUNT(*) > 1" )
                    .arg( this->fieldName( Task ),
                          this->fieldName( Team ),
                          this->tableName(),
                          QString::number( teamId )));

        while ( query.next()) {
            const int count = query.value( 2 ).toInt();
            const int team = query.value( 1 ).toInt();
            const int task = query.value( 0 ).toInt();
            QSqlQuery subQuery;

            // announce the total amount of duplicate logs
            qCDebug( Database_::Debug ) << this->tr( "performing deletion of %1 duplicate logs from team %2 for task %3" )
                                           .arg( QString::number( count ),
                                                 Team::instance().title( Team::instance().row( static_cast<Id>( team ))),
                                                 Task::instance().name( Task::instance().row( static_cast<Id>( task ))));

            // delete actual logs
            subQuery.exec( QString( "DELETE FROM %1 WHERE %2=%3 AND %4=%5" )
                           .arg( this->tableName(),
                                 this->fieldName( Team ),
                                 QString::number( teamId ),
                                 this->fieldName( Task ),
                                 QString::number( task )));
        }
    }

    // removing orphaned combos here
    {
        query.exec( QString( "SELECT %1, COUNT(*) FROM %2 WHERE %1>-1 GROUP BY %1 HAVING COUNT(*) = 1" )
                    .arg( this->fieldName( Combo ), this->tableName()));

        while ( query.next()) {
            QSqlQuery subQuery;

            const int combo = query.value( 0 ).toInt();
            qCDebug( Database_::Debug ) << this->tr( "clearing an orphaned combo with id:%1" ).arg( combo );
            subQuery.exec( QString( "UPDATE %1 SET %2=-1 WHERE %2=%3" )
                           .arg( Log::instance().tableName(),
                                 Log::instance().fieldName( Log::Combo ),
                                 QString::number( combo )));
        }
    }

    // select the updated table
    Log::instance().select();
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
                .arg( this->fieldName( Log::ID ),
                      this->fieldName( Log::Task ),
                      this->fieldName( Log::Team ),
                      this->tableName(),
                      this->fieldName( Log::Task ),
                      QString::number( static_cast<int>( taskId )),
                      this->fieldName( Log::Team ),
                      QString::number( static_cast<int>( teamId ))));

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
            Log::instance().remove( row );
        else
            Log::instance().setMultiplier( row, multi );

#ifdef QT_DEBUG
        qCDebug( Database_::Debug ) << ( multi <= 0 ? "delete" : "change" ) << "log at row" << row;
#endif
    } else {
#ifdef QT_DEBUG
        qCDebug( Database_::Debug ) << "new log";
#endif
        Log::instance().add( taskId, teamId, multi );
    }
}
