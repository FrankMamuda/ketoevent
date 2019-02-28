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
#include <QSqlQuery>

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
    /* addme */ //this->addField( Tag,   "tag", QVariant::String, "varchar( 64 )" );
    // tag will be used to flag special tasks in javascript, so that tags remain
    // constant even if name or other attributes change
    //
    // this could be used for special tasks such as finish time
    // tag also would allow special formatting from js side
    //

    // map types and styles
    this->types[Types::Check]     = QObject::tr( "Check" );
    this->types[Types::Multi]     = QObject::tr( "Multi" );
    this->styles[Styles::Regular] = QObject::tr( "Regular" );
    this->styles[Styles::Bold]    = QObject::tr( "Bold" );
    this->styles[Styles::Italic]  = QObject::tr( "Italic" );

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

#ifdef QT_DEBUG
    if ( role == Qt::DisplayRole ) {
        const Row row = this->row( index.row());
        const bool isMulti = this->type( row ) == Task::Types::Multi;

        return QString( "%1 (%2%3)")
                .arg( this->name( row ))
                .arg( isMulti ? this->tr( "M" ) : this->tr( "R" ))
                .arg( isMulti ? QString( ", %1x%2" )
                                .arg( this->points( row ))
                                .arg( this->multi( row )) :
                                QString( ", %1" ).arg( this->points( row )));
    }
#endif

    return Table::data( index, role );
}

/**
 * @brief Task::multiplier
 * @param row
 * @return
 */
int Task::multiplier( const Row &row ) const {
    bool ok;
    const QPair<Id, Id> ids( getIds( row, &ok ));

    return ok ? Log::instance()->multiplier( ids.first, ids.second ) : 0;
}

/**
 * @brief Task::comboId
 * @param row
 * @return
 */
Id Task::comboId( const Row &row ) const {
    bool ok;
    const QPair<Id, Id> ids( getIds( row, &ok ));

    return ok ? Log::instance()->comboId( ids.first, ids.second ) : Id::Invalid;

    // NOTE: could replace with a value from extended (LOG) table (could increase performance marginally)
    //return static_cast<Id>( this->value( row, ComboID ).toInt());
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
 * @brief Task::removeOrphanedEntries
 */
void Task::removeOrphanedEntries() {
    QSqlQuery query;

    // remove orphaned tasks
    query.exec( QString( "delete from %1 where %2 not in (select %3 from %4)" )
                .arg( this->tableName())
                .arg( this->fieldName( Event ))
                .arg( Event::instance()->fieldName( Event::ID ))
                .arg( Event::instance()->tableName()));
    this->select();
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

/**
 * @brief Task::selectStatement
 * @return
 */
QString Task::selectStatement() const {
    int y;
    QString statement;

    // return default statment if database has not been initialized
    if ( !Database::instance()->hasInitialised() || !this->hasInitialised())
        return Table::selectStatement();

    // validate team row
    const Row team = MainWindow::instance()->currentTeam();
    if ( team == Row::Invalid )
        return statement;

    // validate team id
    const Id teamId = Team::instance()->id( team );
    if ( teamId == Id::Invalid )
        return statement;

    // get table name from LOGS
    const QString logs( Log::instance()->tableName());
    if ( logs.isEmpty())
        return statement;

    // we are making a different statement to include logs
    statement = QString( "SELECT" );

    // add fields from TASK table
    for ( y = 0; y < Task::Fields::Count; y++ ) {
        const QString field( this->fieldName( y ));
        if ( field.isEmpty())
            return "";

        statement.append( QString( " %1.%2," ).arg( this->tableName()).arg( field ));
    }

    // append fields from LOG table
    statement.append( QString( " %1.%2," ).arg( logs ).arg( Log::instance()->fieldName( Log::Fields::Multi )));
    statement.append( QString( " %1.%2" ).arg( logs ).arg( Log::instance()->fieldName( Log::Fields::Combo )));

    // append table name
    statement.append( QString( " FROM %1" ).arg( this->tableName()));

    // FROM table statement
    const QString leftTable( QString( "( SELECT * FROM %1 WHERE %1.%2=%3 GROUP BY %1.%4 ) AS %1" )
                             .arg( logs )
                             .arg( Log::instance()->fieldName( Log::Fields::Team ))
                             .arg( static_cast<int>( teamId ))
                             .arg( Log::instance()->fieldName( Log::Fields::Task )));

    // append JOIN statement from LOGS table
    statement.append( QString( " LEFT JOIN %1 ON %2.%3=%4.%5" )
                      .arg( leftTable )
                      .arg( logs )
                      .arg( Log::instance()->fieldName( Log::Fields::Task ))
                      .arg( this->tableName())
                      .arg( this->fieldName( ID )));

    // append filter if any
    if ( !this->filter().isEmpty())
        statement.append( QString( " WHERE %1" ).arg( this->filter()));

    // remove trailing whitespace
    statement = statement.simplified();

    // DEBUG:
    //qDebug() << statement;

    return statement;
}
