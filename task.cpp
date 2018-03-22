/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "log.h"
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

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
    this->connect( this, SIGNAL( changed()), Main::instance(), SLOT( update()));
}

/**
 * @brief Task::calculate
 * @param logId
 * @return
 */
int Task::calculate( int logId ) const {
    int value = 0;

    // get log parent
    Log *logPtr = Log::forId( logId );
    if ( logPtr == nullptr )
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
    QSqlQuery query;
    int max = 0;

    // announce
    Common::print( CLMsg + QObject::tr( "adding a new task - name - '%1'; points - %2; multi - %3; type - %4; style - %5; description - '%6'\n" )
             .arg( taskName )
             .arg( points )
             .arg( multi )
             .arg( type )
             .arg( style )
             .arg( description ),
             Common::TaskDebug );

    // avoid duplicates
    if ( Task::forName( taskName ) != nullptr )
        return;

    // make sure we insert value at the bottom of the list
    query.exec( "select max ( parent ) from tasks" );
    while ( query.next())
        max = query.value( 0 ).toInt();

    // perform database update and select last row
    query.prepare( "insert into tasks values ( null, :name, :points, :multi, :style, :type, :parent, :eventId, :description )" );
    query.bindValue( ":name", taskName );
    query.bindValue( ":points", points );
    query.bindValue( ":multi", multi );
    query.bindValue( ":style", static_cast<Task::Styles>( style ));
    query.bindValue( ":type", static_cast<Task::Types>( type ));
    query.bindValue( ":parent", max + 1 );
    query.bindValue( ":eventId", Event::active()->id());
    query.bindValue( ":description", description );

    if ( !query.exec()) {
        Common::error( CLSoftError, QObject::tr( "could not add task, reason: %1\n" ).arg( query.lastError().text()));
        return;
    }

    // select the new entry
    query.exec( QString( "select * from tasks where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next())
        Main::instance()->taskList << new Task( query.record(), "tasks" );

    // add to event
    Event::active()->taskList << Main::instance()->taskList.last();
}

/**
 * @brief Task::loadTasks
 */
void Task::loadTasks() {
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "loading tasks from database\n" ), Common::TaskDebug );

    // read all task entries
    query.exec( "select * from tasks order by parent asc" );

    // store entries
    while ( query.next())
        Main::instance()->taskList << new Task( query.record(), "tasks" );
}

/**
 * @brief Task::forId
 * @param id
 * @return
 */
Task *Task::forId( int id ) {
    // search current event ONLY
    foreach ( Task *taskPtr, Event::active()->taskList /*this->base.taskList*/ ) {
        if ( taskPtr->id() == id )
            return taskPtr;
    }
    return nullptr;
}

/**
 * @brief Task::forName
 * @param name
 * @param currentEvent
 * @return
 */
Task *Task::forName( const QString &name, bool currentEvent ) {
    QList <Task*> taskList;

    if ( currentEvent )
        taskList = Event::active()->taskList;
    else
        taskList = Main::instance()->taskList;

    // search current event ONLY by default
    foreach ( Task *taskPtr, taskList ) {
        if ( !QString::compare( name, taskPtr->name()))
            return taskPtr;
    }
    return nullptr;
}
