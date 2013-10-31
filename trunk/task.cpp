/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// task.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
addTask
================
*/
void Main::addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style ) {
    QSqlQuery query;
    int max = 0;

    // avoid duplicates
    if ( m.taskForName( taskName ) != NULL )
        return;

    // make sure we insert value at the bottom of the list
    query.exec( "select max ( parent ) from tasks" );
    while ( query.next())
        max = query.value( 0 ).toInt();

    // perform database update and select last row
    if ( !query.exec( QString( "insert into tasks values ( null, '%1', %2, %3, %4, %5, %6 )" )
                      .arg( taskName )
                      .arg( points )
                      .arg( multi )
                      .arg( static_cast<TaskEntry::Styles>( style ))
                      .arg( static_cast<TaskEntry::Types>( type ))
                      .arg( max + 1 )
                      )) {
        this->error( StrSoftError + QString( "could not add task, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from tasks where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next())
        this->taskList << new TaskEntry( query.record(), "tasks" );
}

/*
================
loadTasks
================
*/
void Main::loadTasks() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from tasks order by parent asc" );

    // store entries
    while ( query.next())
        this->taskList << new TaskEntry( query.record(), "tasks" );
}

/*
================
taskForId
================
*/
TaskEntry *Main::taskForId( int id ) {
    foreach ( TaskEntry *taskPtr, this->taskList ) {
        if ( taskPtr->id() == id )
            return taskPtr;
    }
    return NULL;
}

/*
================
taskForName
================
*/
TaskEntry *Main::taskForName( const QString &name ) {
    foreach ( TaskEntry *taskPtr, this->taskList ) {
        if ( !QString::compare( name, taskPtr->name()))
            return taskPtr;
    }
    return NULL;
}
