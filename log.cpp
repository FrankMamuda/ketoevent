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

    // set relation
    this->setRelation( Task, QSqlRelation( TaskTable::Name, Task::instance()->field( Task::ID )->name(), Task::instance()->field( Task::Name )->name()));
    this->setRelation( Team, QSqlRelation( TeamTable::Name, Team::instance()->field( Team::ID )->name(), Team::instance()->field( Team::Title )->name()));
}

/**
 * @brief Log::add
 * @param taskId
 * @param teamId
 * @param multiplier
 * @param comboId
 */
void Log::add( int taskId, int teamId, int multiplier, int comboId ) {
    Table::add( QVariantList() <<
                Database_::null <<
                multiplier <<
                taskId <<
                teamId <<
                comboId );
}

/**
 * @brief Log::data
 * @param item
 * @param role
 * @return
 */
QVariant Log::data( const QModelIndex &item, int role ) const {
    if ( role == MultiRole )
        return this->multiplier( item.row());

    if ( role == TaskRole )
        return this->task( item.row());

    return Table::data( item, role );
}

/**
 * @brief Log::task
 * @param row
 * @return
 */
int Log::task( int row ) const {
    return Task::instance()->row( this->taskId( row ));
}

/**
 * @brief Log::team
 * @param row
 * @return
 */
int Log::team( int row ) const {
    return Team::instance()->row( this->teamId( row ));
}
