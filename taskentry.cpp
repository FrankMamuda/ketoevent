/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "taskentry.h"
#include "logentry.h"
#include "main.h"

/**
 * @brief Task::Task
 * @param record
 * @param table
 */
Task::Task( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );

    // points failsafe
    if ( this->points() < 0 )
        this->setPoints();

    // maximum multiplier failsafe
    if ( this->multi() < 0 )
        this->setMulti();

    // type failsafe
    switch ( this->type()) {
    case Check:
    case Multi:
        break;

    default:
        this->setType();
    }

    // set defaults
    this->m_reindex = false;
    this->m_order = this->order( true );

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/**
 * @brief Task::calculate
 * @param logId
 * @return
 */
int Task::calculate( int logId ) const {
    int value = 0;

    // get log parent
    Log *logPtr = m.logForId( logId );
    if ( logPtr == NULL )
        return 0;

    // get initial points
    switch ( this->type()) {
    case Check:
        if ( logPtr->value())
            value += this->points();
        break;

    case Multi:
        if ( logPtr->value() > this->multi())
            value += this->points() * this->multi();
        else
            value += this->points() * logPtr->value();
        break;

    default:
        // error
        return 0;
    }

    // we're done
    return value;
}

/**
 * @brief Task::setOrder
 * @param order
 * @param direct
 */
void Task::setOrder( int order, bool direct ) {
    if ( order < 0 )
        return;

    if ( direct ) {
        this->setValue( "parent", order );
        this->m_reindex = false;
        this->m_order = order;
    } else {
        this->m_order = order;
        this->m_reindex = true;
    }
}

/**
 * @brief Task::order
 * @param sql
 * @return
 */
int Task::order( bool sql ) const {
    if ( sql )
        return this->record().value( "parent" ).toInt();

    return this->m_order;
}
