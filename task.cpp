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
#include "mainwindow.h"
#include "log.h"
#include "team.h"
#include <QFont>

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

    // map types and styles
    this->types[Types::Check]     = QT_TR_NOOP_UTF8( "Check" );
    this->types[Types::Multi]     = QT_TR_NOOP_UTF8( "Multi" );
    this->styles[Styles::Regular] = QT_TR_NOOP_UTF8( "Regular" );
    this->styles[Styles::Bold]    = QT_TR_NOOP_UTF8( "Bold" );
    this->styles[Styles::Italic]  = QT_TR_NOOP_UTF8( "Italic" );

    // sort by order
    this->setSort( Task::Order, Qt::AscendingOrder );
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
Row Task::add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style, const QString &description ) {
    int y, highest = -1;

    // failsafe
    const Row event = MainWindow::instance()->currentEvent();
    if ( event == Row::Invalid ) {
        qDebug() << this->tr( "no active event, aborting" );
        return Row::Invalid;
    }

    // find highest order
    for ( y = 0; y < this->count(); y++ )
        highest = qMax( highest, this->order( this->row( y )));

    // add a new team
    return Table::add( QVariantList() <<
                Database_::null <<
                taskName <<
                points <<
                multi <<
                static_cast<int>( style ) <<
                static_cast<int>( type ) <<
                highest + 1 <<
                static_cast<int>( Event::instance()->id( event )) <<
                description );
}

/**
 * @brief Task::data
 * @param idx
 * @param role
 * @return
 */
QVariant Task::data( const QModelIndex &index, int role ) const {
    const Row row = this->row( index );

    if ( role == Qt::FontRole ) {
        QFont font( Table::data( index, Qt::FontRole ).value<QFont>());

        if ( Task::instance()->style( row ) == Styles::Italic ) {
            font.setItalic( true );
            return font;
        }

        if ( Task::instance()->style( row ) == Styles::Bold ) {
            font.setBold( true );
            return font;
        }
    }

    return Table::data( index, role );
}

/**
 * @brief Task::multiplier
 * @param row
 * @return
 */
int Task::multiplier( const Row &row ) const {
    bool ok;
    QPair<Id, Id> ids( getIds( row, &ok ));

    return ok ? Log::instance()->multiplier( ids.first, ids.second ) : 0;
}

/**
 * @brief Task::comboId
 * @param row
 * @return
 */
Id Task::comboId( const Row &row ) const {
    bool ok;
    QPair<Id, Id> ids( getIds( row, &ok ));

    return ok ? Log::instance()->comboId( ids.first, ids.second ) : Id::Invalid;
}

/**
 * @brief Task::getIds
 * @param row
 * @param ok
 * @return
 */
QPair<Id, Id> Task::getIds( const Row &row, bool *ok ) const {
    QPair<Id, Id> out( Id::Invalid, Id::Invalid );
    *ok = false;

    if ( row == Row::Invalid )
        return out;

    const Row team = MainWindow::instance()->currentTeam();
    if ( team == Row::Invalid )
        return out;

    out.second = Team::instance()->id( team );
    if ( out.second == Id::Invalid )
        return out;

    out.first = this->id( row );
    if ( out.first == Id::Invalid )
        return out;

    *ok = true;
    return out;
}

/**
 * @brief Task::setMultiplier
 * @param row
 * @param value
 */
void Task::setMultiplier( const Row &row, int value ) {
    bool ok;
    QPair<Id, Id> ids( getIds( row, &ok ));

    if ( ok )
        Log::instance()->setMultiplier( value, ids.first, ids.second );
}
