/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
void Main::addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style, const QString &description ) {
    QSqlQuery query;
    int max = 0;

    // avoid duplicates
    if ( this->taskForName( taskName ) != NULL )
        return;

    // make sure we insert value at the bottom of the list
    query.exec( "select max ( parent ) from tasks" );
    while ( query.next())
        max = query.value( 0 ).toInt();

    // perform database update and select last row
    //   id integer primary key, name varchar( 128 ), points integer, multi integer, style integer,
    //   type integer, parent integer, eventId integer, description varchar( 512 )
    if ( !query.exec( QString( "insert into tasks values ( null, '%1', %2, %3, %4, %5, %6, %7, '%8' )" )
                      .arg( taskName )
                      .arg( points )
                      .arg( multi )
                      .arg( static_cast<TaskEntry::Styles>( style ))
                      .arg( static_cast<TaskEntry::Types>( type ))
                      .arg( max + 1 )
                      .arg( m.currentEvent()->id())
                      .arg( description )
                      )) {
        this->error( StrSoftError, QString( "could not add task, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from tasks where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next())
        this->base.taskList << new TaskEntry( query.record(), "tasks" );

    // add to event
    this->currentEvent()->taskList << this->base.taskList.last();
}

/*
================
loadTasks
================
*/
void Main::loadTasks( bool import, bool store ) {
    QSqlQuery query;

    // read stuff
    if ( import )
        query.exec( "select * from merge.tasks order by parent asc" );
    else
        query.exec( "select * from tasks order by parent asc" );

    // store entries
    while ( query.next()) {
        TaskEntry *taskPtr = new TaskEntry( query.record(), "tasks" );

        // since we're just checking hash and not adding any new tasks on import
        // there is no need for reindexing
        if ( import ) {
            taskPtr->setImported();
            this->import.taskList << taskPtr;
        } else
            this->base.taskList << taskPtr;
    }

    // handle importing
    if ( import ) {
        bool duplicate = false;

        // check for duplicates
        foreach ( TaskEntry *importedTaskPtr, this->import.taskList ) {
            duplicate = false;

            foreach ( TaskEntry *taskPtr, this->base.taskList ) {
                // there's a match
                if ( !QString::compare( taskPtr->name(), importedTaskPtr->name())) {
                    // first time import, just append imported
                    if ( !importedTaskPtr->name().endsWith( " (imported)")) {
                        importedTaskPtr->setName( importedTaskPtr->name() + " (imported)" );
                    }
                    // second time import is a no-go
                    else {
                        duplicate = true;
                        m.error( StrSoftError, this->tr( "aborting double import of task \"%1\"\n" ).arg( importedTaskPtr->name()));
                        this->import.taskList.removeOne( taskPtr );
                        continue;
                    }
                }
            }

            // store the new-found team
            if ( !duplicate && store )
                importedTaskPtr->store();
        }
    }
}

/*
================
taskForId
================
*/
TaskEntry *Main::taskForId( int id ) {
    foreach ( TaskEntry *taskPtr, this->base.taskList ) {
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
    foreach ( TaskEntry *taskPtr, this->base.taskList ) {
        if ( !QString::compare( name, taskPtr->name()))
            return taskPtr;
    }
    return NULL;
}