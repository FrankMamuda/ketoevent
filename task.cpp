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
#include "task.h"
#include "database.h"
#include "event.h"
#include "mainwindow.h"
#include "log.h"
#include "team.h"
#include <QFont>
#include <QSqlQuery>

// singleton
Task *Task::i = nullptr;

#define stringify( x ) #x

/**
 * @brief Task::Task
 */
Task::Task() : Table( "tasks" ) {
    PRIMARY_FIELD( ID );
    FIELD( Name,     QMetaType::QString );
    FIELD( Points,   QMetaType::Int );
    FIELD( Mult,     QMetaType::Int );
    FIELD( Style,    QMetaType::Int );
    FIELD( Type,     QMetaType::Int );
    FIELD( Order_,   QMetaType::Int );
    FIELD( Event,    QMetaType::Int );
    FIELD( Desc,     QMetaType::QString );
    FIELD( Pattern,  QMetaType::QString );
    this->addUniqueConstraint( QStringList() << IDTOFIELD( Name ) << IDTOFIELD( Event ));

    // map types and styles
    this->types[Types::Check]     = QObject::tr( "Check" );
    this->types[Types::Multi]     = QObject::tr( "Multi" );
    this->styles[Styles::Regular] = QObject::tr( "Regular" );
    this->styles[Styles::Bold]    = QObject::tr( "Bold" );
    this->styles[Styles::Italic]  = QObject::tr( "Italic" );

    // sort by order
    this->setSort( Task::Order_, Qt::AscendingOrder );
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
Row Task::add( const QString &taskName, int points, int multi, Task::Types type,
               Task::Styles style, const QString &description, const QString &pattern ) {
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

    // add a new task
    return Table::add( QVariantList() <<
                       Database_::null <<
                       taskName <<
                       points <<
                       multi <<
                       static_cast<int>( style ) <<
                       static_cast<int>( type ) <<
                       highest + 1 <<
                       static_cast<int>( Event::instance()->id( event )) <<
                       description <<
                       pattern
                       );
}

/**
 * @brief Task::data
 * @param idx
 * @param role
 * @return
 */
QVariant Task::data( const QModelIndex &index, int role ) const {
    const Row row = this->row( index );

    if ( role == Qt::FontRole && index.isValid() && row != Row::Invalid && index.column() == Task::Name ) {
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

    if ( role == Qt::DisplayRole && index.column() == Task::Type && row != Row::Invalid ) {
        if ( Task::instance()->type( row ) == Types::Check )
            return Task::tr( "\u2713" );
        if ( Task::instance()->type( row ) == Types::Multi )
            return Task::tr( "Multi" );
    }

    if ( role == Qt::DisplayRole && index.column() == Task::Mult && row != Row::Invalid ) {
        if ( Task::instance()->type( row ) != Types::Multi )
            return "";
        if ( Task::instance()->type( row ) == Types::Multi )
            return QString( "%1\u00d7%2=%3" ).arg( QString::number( Task::instance()->points( row )), QString::number( Task::instance()->multi( row )), QString::number( Task::instance()->points( row ) * Task::instance()->multi( row )));
    }

    if ( role == Qt::TextAlignmentRole && index.column() != Task::Name && index.column() != Task::Desc )
        return Qt::AlignCenter;

    return Table::data( index, role );
}


/**
 * @brief Task::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant Task::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( role == Qt::DisplayRole ) {
        switch ( section ) {

        case Name: return Event::tr( "Name" );
        case Points: return Event::tr( "Points" );
        case Mult: return Event::tr( "Multi" );
        case Desc: return Event::tr( "Description" );
        case Type: return Event::tr( "Type" );

        default:
        case Style:
        case Order_:
        case Event:
        case ID:
            break;
        }
    }

    if ( role == Qt::TextAlignmentRole )
        return Qt::AlignCenter;

    if ( role == Qt::FontRole ) {
        QFont font;
        font.setBold( true );
        return font;
    }

    return Table::headerData( section, orientation, role );
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
    query.exec( QString( "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)" )
                .arg( this->tableName(),
                      this->fieldName( Event ),
                      Event::instance()->fieldName( Event::ID ),
                      Event::instance()->tableName()));
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

        statement.append( QString( " %1.%2," ).arg( this->tableName(), field ));
    }

    // append fields from LOG table
    statement.append( QString( " %1.%2," ).arg( logs, Log::instance()->fieldName( Log::Fields::Multi )));
    statement.append( QString( " %1.%2" ).arg( logs, Log::instance()->fieldName( Log::Fields::Combo )));

    // append table name
    statement.append( QString( " FROM %1" ).arg( this->tableName()));

    // FROM table statement
    const QString leftTable( QString( "( SELECT * FROM %1 WHERE %1.%2=%3 GROUP BY %1.%4 ) AS %1" )
                             .arg( logs,
                                   Log::instance()->fieldName( Log::Fields::Team ),
                                   QString::number( static_cast<int>( teamId )),
                                   Log::instance()->fieldName( Log::Fields::Task )));

    // append JOIN statement from LOGS table
    statement.append( QString( " LEFT JOIN %1 ON %2.%3=%4.%5" )
                      .arg( leftTable,
                            logs,
                            Log::instance()->fieldName( Log::Fields::Task ),
                            this->tableName(),
                            this->fieldName( ID )));

    // append filter if any
    if ( !this->filter().isEmpty())
        statement.append( QString( " WHERE %1" ).arg( this->filter()));

    // remove trailing whitespace
    statement = statement.simplified();

    return statement;
}
