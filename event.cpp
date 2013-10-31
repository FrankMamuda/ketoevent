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
void Main::addEvent() {
    QSqlQuery query;
    QString comboString;
    QString timeString;
    int count = 0;

    // currenly we have only one entry
    // in future use multiple databases for multiple events
    if ( this->event != NULL ) {
        m.error( StrSoftError + QString( "event '%1' already present, aborting\n" ).arg( this->event->name()));
        return;
    }

    // compile strings
    comboString = QString( "%1, %2, %3" )
            .arg( Common::defaultSingleCombo )
            .arg( Common::defaultDoubleCombo )
            .arg( Common::defaultTripleCombo );
    timeString = QString( "'%1', '%2', '%3'" )
            .arg( Common::defaultStartTime )
            .arg( Common::defaultFinishTime )
            .arg( Common::defaultFinalTime );

    // add new log
    if ( !query.exec( QString( "insert into events values ( 1, %1, '%2', %3, %4, %5, %6, %7 )" )
                      .arg( Common::API )
                      .arg( this->tr( "unnamed event" ))
                      .arg( Common::defaultMinMembers )
                      .arg( Common::defaultMaxMembers )
                      .arg( timeString )
                      .arg( Common::defaultPenaltyPoints )
                      .arg( comboString ))) {
        this->error( StrSoftError + QString( "could not add event, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from events where id=1" ));

    // get last entry and construct internal entry
    while ( query.next()) {
        this->event = new EventEntry( query.record(), "events" );
        count++;
        break;
    }

    // fail
    if ( count == 0 )
        m.error( StrFatalError + "could not create event\n" );
}


/*
================
loadEvent
================
*/
void Main::loadEvent() {
    QSqlQuery query;
    int count = 0;

    // currently read the first entry
    query.exec( "select * from events where id=1" );

    // store entries
    while ( query.next()) {
        this->event = new EventEntry( query.record(), "events" );

        // failsafe - api check
        // add compatibility in future if needed (unlikely)
        // TODO: add dialog to create new database (rename old one)
        if ( static_cast<unsigned int>( event->api()) < Common::MinimumAPI ) {
            m.error( StrFatalError +
                     QString( "incompatible API - '%1', minimum supported %2\n" )
                     .arg( event->api())
                     .arg( Common::MinimumAPI ));
        }
        count++;
    }

    // no event entry? - create one
    if ( count == 0 )
        this->addEvent();
}
