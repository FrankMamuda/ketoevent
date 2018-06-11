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
#include "team.h"
#include "field.h"
#include "database.h"
#include "event.h"
#include "mainwindow.h"

//
// namespaces
//
using namespace TeamTable;

/**
 * @brief Team::Team
 */
Team::Team() : Table( TeamTable::Name ) {
    this->addField( ID,       "id",         QVariant::UInt,   "integer primary key", true, true );
    this->addField( Title,    "name",       QVariant::String, "varchar( 64 )",       true );
    this->addField( Members,  "members",    QVariant::Int,    "integer" );
    this->addField( Finish,   "finishTime", QVariant::String, "varchar( 5 )" );
    this->addField( Lock,     "lock",       QVariant::Int,    "integer" );
    this->addField( Reviewer, "reviewer",   QVariant::String, "varchar( 64 )" );
    this->addField( Event,    "eventId",    QVariant::Int,    "integer" );

    // set relation
    this->setRelation( Event, QSqlRelation( EventTable::Name, Event::instance()->field( Event::ID )->name(), Event::instance()->field( Event::Title )->name()));
}

/**
 * @brief Team::add
 * @param title
 * @param members
 * @param finishTime
 * @param reviewer
 */
void Team::add( const QString &title, int members, const QTime &finishTime, const QString &reviewer ) {
    Table::add( QVariantList() <<
                Database_::null <<
                title <<
                members <<
                finishTime.toString( "hh:mm" ) <<
                0 <<
                reviewer <<
                MainWindow::instance()->currentEventId());

    // FIXME: does not work!!
    this->submitAll();
}
