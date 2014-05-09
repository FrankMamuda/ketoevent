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
// event.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
addEvent
================
*/
void Main::addEvent( const QString &title ) {
    QSqlQuery query;
    QString comboString;
    QString timeString;
    QString eventTitle;

    // copy title
    eventTitle = title;

    // set title if none
    if ( eventTitle.isEmpty())
        eventTitle = this->tr( "unnamed event" );

    // compile strings
    comboString = QString( "%1, %2, %3" )
            .arg( Common::defaultComboOfTwo )
            .arg( Common::defaultComboOfThree )
            .arg( Common::defaultComboOfFourAndMore );
    timeString = QString( "'%1', '%2', '%3'" )
            .arg( Common::defaultStartTime )
            .arg( Common::defaultFinishTime )
            .arg( Common::defaultFinalTime );

    // add new event log with default/built-in values
    if ( !query.exec( QString( "insert into events values ( null, %1, '%2', %3, %4, %5, %6, %7, '0' )" )
                      .arg( Common::API )
                      .arg( eventTitle )
                      .arg( Common::defaultMinMembers )
                      .arg( Common::defaultMaxMembers )
                      .arg( timeString )
                      .arg( Common::defaultPenaltyPoints )
                      .arg( comboString ))) {
        this->error( StrSoftError + QString( "could not add event, reason - \"%1\"\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from events where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last sql entry and construct internal entry
    while ( query.next()) {
        this->eventList << new EventEntry( query.record(), "events" );
        break;
    }
}

/*
================
loadEvents
================
*/
void Main::loadEvents() {
    QSqlQuery query;

    // read all event entries
    query.exec( "select * from events" );

    // store entries
    while ( query.next()) {
        this->eventList << new EventEntry( query.record(), "events" );

        // failsafe - api check
        // add compatibility in future if needed (unlikely)
        if ( static_cast<unsigned int>( this->eventList.last()->api()) < Common::MinimumAPI ) {
            this->error( StrSoftError +
                     this->tr( "incompatible API - '%1', minimum supported '%2'\n" )
                     .arg( this->eventList.last()->api())
                     .arg( Common::MinimumAPI ));
            this->eventList.removeLast();
        }
    }

    // no event entry? - create one
    if ( this->eventList.isEmpty())
        this->addEvent();

    // still nothing?
    if ( this->eventList.isEmpty()) {
        this->error( StrFatalError + this->tr( "could not create event\n" ));
    }

    // for now - resort to indexes?? (use list indexof)
    this->setCurrentEvent( this->eventList.last());
}

/*
================
currentEvent
================
*/
EventEntry *Main::currentEvent() {
    if ( m_event == NULL )
        this->error( StrFatalError + this->tr( "no valid events\n" ));

    return this->m_event;
}

/*
================
setCurrentEvent
================
*/
bool Main::setCurrentEvent( EventEntry *eventPtr ) {
    foreach ( EventEntry *entry, this->eventList ) {
        if ( entry == eventPtr ) {
            this->m_event = entry;
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
    foreach ( EventEntry *eventPtr, this->eventList ) {
        if ( eventPtr->id() == id )
            return eventPtr;
    }
    return NULL;
}
