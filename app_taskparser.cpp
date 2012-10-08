/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
// includes
//
#include "app_taskparser.h"
#include "app_taskentry.h"
#include "app_main.h"

/*
================
startElement
================
*/
bool App_TaskParser::startElement( const QString &, const QString &, const QString &name, const QXmlAttributes &attributes ) {
    int y;

    if ( !QString::compare( name, "tasks" ))
        this->setState( Document );
    else if ( !QString::compare( name, "task" ))
        this->setState( Tasks );
    else {
        com.error( Sys_Common::SoftError, QObject::trUtf8( "App_TaskParser::startElement: unknown element \"%1\"\n" ).arg( name ));
        return false;
    }

    if ( this->state() == Document ) {
        // do smth
    } else if ( this->state() == Tasks ) {
        // begin parsing log entry
        taskData_t task;

        // set defaults
        task.name = QString::null;
        task.type = App_TaskEntry::NoType;
        task.points = task.maxMulti = 0;
        task.challenge = false;

        // get attributes
        for ( y = 0; y < attributes.count(); y++ ) {
            if ( !QString::compare( attributes.qName( y ), "name" ))
                task.name = attributes.value( y );
            else if ( !QString::compare( attributes.qName( y ), "type" ))
                task.type = static_cast<App_TaskEntry::Types>( attributes.value( y ).toInt());
            else if ( !QString::compare( attributes.qName( y ), "points" ))
                task.points = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "max" ))
                task.maxMulti = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "challenge" ))
                task.challenge = static_cast<bool>( attributes.value( y ).toInt());
        }

        // failsafe
        if ( task.name.isEmpty()) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "App_TaskParser::startElement: task name is empty\n" ));
            return false;
        }

        this->taskData << task;
    }
    return true;
}

/*
================
startElement
================
*/
bool App_TaskParser::endElement( const QString &, const QString &, const QString &name ) {
    if ( !QString::compare( name, "tasks" )) {
        // add logs to team data
        foreach ( taskData_t task, this->taskData ) {
            // failsafe type
            switch ( task.type ) {
            case App_TaskEntry::Check:
            case App_TaskEntry::Multi:
            case App_TaskEntry::Special:
                break;

            case App_TaskEntry::NoType:
            default:
                com.error( Sys_Common::SoftError, QObject::trUtf8( "App_TaskParser::endElement: unknown task with type \"%1\"\n" ).arg( static_cast<int>( task.type )));
                return false;
            }

            // finally add the new task
            m.addTask( new App_TaskEntry( task.name, task.type, task.points, task.maxMulti, task.challenge ));
        }
    } else if ( !QString::compare( name, "task" )) {
        this->setState( Document );
    } else {
        com.error( Sys_Common::SoftError, QObject::trUtf8( "App_TaskParser::endElement: unknown element \"%1\"\n" ).arg( name ));
        return false;
    }

    return true;
}
