/*
===========================================================================
Copyright (C) 2013-2016 Avotu Briezhaudzetava

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
// event.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

/*
================
addEvent
================
*/
void Main::addEvent( const QString &title ) {
    QSqlQuery query;
    QString eventTitle;

    // announce
    m.print( StrMsg + this->tr( "adding a new event - '%1'\n" ).arg( title ), Main::Event );

    // copy title
    eventTitle = title;

    // set title if none
    if ( eventTitle.isEmpty())
        eventTitle = this->tr( "unnamed event" );

    // add new event log with default/built-in values
#ifdef SQL_PREPARE_STATEMENTS
    query.prepare( QString( "insert into events values ( null, :api, :name, :minMembers, :maxMembers, :startTime, :finishTime, :finalTime, :penalty, :comboOfTwo, :comboOfThree, :comboOfFourAndMore, :lock )" ));
    query.bindValue( ":api", Common::API );
    query.bindValue( ":name", eventTitle );
    query.bindValue( ":minMembers", Common::defaultMinMembers );
    query.bindValue( ":maxMembers", Common::defaultMaxMembers );
    query.bindValue( ":startTime", Common::defaultStartTime );
    query.bindValue( ":finishTime", Common::defaultFinishTime );
    query.bindValue( ":finalTime", Common::defaultFinalTime );
    query.bindValue( ":penalty", Common::defaultPenaltyPoints );
    query.bindValue( ":comboOfTwo", Common::defaultComboOfTwo );
    query.bindValue( ":comboOfThree", Common::defaultComboOfThree );
    query.bindValue( ":comboOfFourAndMore", Common::defaultComboOfFourAndMore );
    query.bindValue( ":lock", 0 );

    if ( !query.exec())
#else
    QString comboString;
    QString timeString;

    // compile strings
    comboString = QString( "%1, %2, %3" )
            .arg( Common::defaultComboOfTwo )
            .arg( Common::defaultComboOfThree )
            .arg( Common::defaultComboOfFourAndMore );
    timeString = QString( "'%1', '%2', '%3'" )
            .arg( Common::defaultStartTime )
            .arg( Common::defaultFinishTime )
            .arg( Common::defaultFinalTime );


    if ( !query.exec( QString( "insert into events values ( null, %1, '%2', %3, %4, %5, %6, %7, '0' )" )
                      .arg( Common::API )
                      .arg( eventTitle )
                      .arg( Common::defaultMinMembers )
                      .arg( Common::defaultMaxMembers )
                      .arg( timeString )
                      .arg( Common::defaultPenaltyPoints )
                      .arg( comboString )))
#endif
        this->error( StrSoftError, QString( "could not add event, reason - \"%1\"\n" ).arg( query.lastError().text()));

    // select the new entry
    query.exec( QString( "select * from events where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last sql entry and construct internal entry
    while ( query.next()) {
        this->base.eventList << new EventEntry( query.record(), "events" );
        break;
    }
}

/*
================
loadEvents
================
*/
bool Main::loadEvents( bool import ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "loading events from database\n" ), Main::Event );

    // read all event entries
    if ( import )
        query.exec( "select * from merge.events" );
    else
        query.exec( "select * from events" );

    // store entries
    while ( query.next()) {
        EventEntry *eventPtr = new EventEntry( query.record(), "events" );

        if ( import ) {
            eventPtr->setImported();
            this->import.eventList << eventPtr;
        } else
            this->base.eventList << eventPtr;

        // failsafe - api check
        if ( static_cast<unsigned int>( this->base.eventList.last()->api()) < Common::MinimumAPI ) {
            this->error( StrSoftError,
                         this->tr( "incompatible API - '%1', minimum supported '%2'\n" )
                         .arg( this->base.eventList.last()->api())
                         .arg( Common::MinimumAPI ));
            this->base.eventList.removeLast();

            if ( import )
                return false;

            // rename database
            this->unloadDatabase();
            QFile::rename( this->cvar( "databasePath" )->string(), QString( "%1_badAPI_%2.db" ).arg( this->cvar( "databasePath" )->string().remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
            this->makePath( this->cvar( "databasePath" )->defaultValue().toString());
            this->loadDatabase();
            return false;
        }
    }

    if ( !import ) {
        // no event entry? - create one
        if ( this->base.eventList.isEmpty())
            this->addEvent();

        // still nothing?
        if ( this->base.eventList.isEmpty()) {
            this->error( StrFatalError, this->tr( "could not create event\n" ));
        }


        // fixes crash on empty database
        // NOTE: rather ugly code
        EventEntry *eventPtr = this->eventForId( this->cvar( "currentEvent" )->integer());
        if ( eventPtr == NULL )
            eventPtr = this->base.eventList.first();

        // for now - resort to indexes?? (use list indexof)
        if ( !this->setCurrentEvent( eventPtr ))
            this->setCurrentEvent( this->base.eventList.first());
    }

    return true;
}

/*
================
currentEvent
================
*/
EventEntry *Main::currentEvent() {
    if ( m_event == NULL )
        this->error( StrFatalError, this->tr( "no valid events\n" ));

    return this->m_event;
}

/*
================
setCurrentEvent
================
*/
bool Main::setCurrentEvent( EventEntry *eventPtr ) {
    // announce
    m.print( StrMsg + this->tr( "setting '%1' as current event\n" ).arg( eventPtr->name()), Main::Event );

    foreach ( EventEntry *entry, this->base.eventList ) {
        if ( entry == eventPtr ) {
            this->m_event = entry;
            this->cvar( "currentEvent" )->setValue( eventPtr->id());
            return true;
        }
    }
    return false;
}

/*
================
eventForId
================
*/
EventEntry *Main::eventForId( int id ) {
    foreach ( EventEntry *eventPtr, this->base.eventList ) {
        if ( eventPtr->id() == id )
            return eventPtr;
    }

    return NULL;
}

/*
================
buildEventTTList
================
*/
void Main::buildEventTTList() {
    // announce
    m.print( StrMsg + this->tr( "building event TTList\n" ), Main::Event );

    foreach ( EventEntry *eventPtr, this->base.eventList ) {
        eventPtr->teamList.clear();
        eventPtr->taskList.clear();

        foreach ( TeamEntry *teamPtr, this->base.teamList ) {
            if ( teamPtr->eventId() == eventPtr->id())
                eventPtr->teamList << teamPtr;
        }

        foreach ( TaskEntry *taskPtr, this->base.taskList ) {
            if ( taskPtr->eventId() == eventPtr->id())
                eventPtr->taskList << taskPtr;
        }
    }
}
