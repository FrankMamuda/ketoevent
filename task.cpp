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
#include "task.h"
#include "field.h"
#include "database.h"
#include "event.h"

//
// namespaces
//
using namespace TaskTable;

/**
 * @brief Task::Task
 */
Task::Task() : Table( TaskTable::Name ) {
    this->addField( ID,     "id",          QVariant::UInt,   "integer primary key", true, true );
    this->addField( Name,   "name",        QVariant::String, "varchar( 128 )",      true );
    this->addField( Points, "points",      QVariant::Int,    "integer" );
    this->addField( Mult,   "multi",       QVariant::Int,    "integer" );
    this->addField( Style,  "style",       QVariant::Int,    "integer" );
    this->addField( Type,   "type",        QVariant::Int,    "integer" );
    this->addField( Order,  "parent",      QVariant::Int,    "integer" );
    this->addField( Event,  "eventId",     QVariant::Int,    "integer" );
    this->addField( Desc,   "description", QVariant::String, "varchar( 512 )" );

    // set relation
    this->setRelation( Event, QSqlRelation( EventTable::Name, Event::instance()->field( Event::ID )->name(), Event::instance()->field( Event::Title )->name()));
}

/**
 * @brief Task::add
 * @param taskName
 * @param points
 * @param multi
 * @param type
 * @param style
 * @param description
 */
void Task::add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style, const QString &description ) {
    Table::add( QVariantList() <<
                Database_::null <<
                taskName <<
                points <<
                multi <<
                static_cast<int>( style ) <<
                static_cast<int>( type ) <<
                0 <<
                0 <<
                description );
}
