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

#include <QSqlQuery>

//
// namespaces
//
using namespace TeamTable;

/**
 * @brief Team::Team
 */
Team::Team() : Table( TeamTable::Name ) {
    this->addField( ID,       "id",         QVariant::UInt,   "integer primary key", true, true );
    /* rename me*/ this->addField( Title,    "name",       QVariant::String, "varchar( 64 )",       true );
    /* remove me*/ this->addField( Members,  "members",    QVariant::Int,    "integer" );
    /* remove me*/ this->addField( Finish,   "finishTime", QVariant::String, "varchar( 5 )" );
    /* remove me*/ this->addField( Lock,     "lock",       QVariant::Int,    "integer" );
    this->addField( Reviewer, "reviewer",   QVariant::String, "varchar( 64 )" );
    this->addField( Event,    "eventId",    QVariant::Int,    "integer" );
}

/**
 * @brief Team::add
 * @param title
 * @param members
 * @param finishTime
 * @param reviewer
 * @return
 */
Row Team::add( const QString &title, const QString &reviewer ) {
    // failsafe
    const Row event = MainWindow::instance()->currentEvent();
    if ( event == Row::Invalid ) {
        qDebug() << this->tr( "no active event, aborting" );
        return Row::Invalid;
    }

    return Table::add( QVariantList() <<
                Database_::null <<
                title <<
                2 <<
                QString( "11:00" ) <<
                0 <<
                reviewer <<
                       static_cast<int>( Event::instance()->id( event )));
}

/**
 * @brief Team::removeOrphanedEntries
 */
void Team::removeOrphanedEntries() {
    QSqlQuery query;

    // remove orphaned teams
    query.exec( QString( "delete from %1 where %2 not in (select %3 from %4)" )
                .arg( this->tableName())
                .arg( this->fieldName( Event ))
                .arg( Event::instance()->fieldName( Event::ID ))
                .arg( Event::instance()->tableName()));
    this->select();
}
