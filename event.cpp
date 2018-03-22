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
#include "main.h"
#include "event.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

/**
 * @brief Event::Event event entry structure (constructor)
 * @param record bound record
 * @param table bound table
 */
Event::Event( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );
}

/**
 * @brief Event::add adds a new event
 * @param title event title
 */
void Event::add( const QString &title ) {
    QSqlQuery query;
    QString eventTitle;

    // announce
    Common::print( CLMsg + QObject::tr( "adding a new event - '%1'\n" ).arg( title ), Common::EventDebug );

    // copy title
    eventTitle = title;

    // set title if none
    if ( eventTitle.isEmpty())
        eventTitle = QObject::tr( "unnamed event" );

    // add new event log with default/built-in values
    query.prepare( QString( "insert into events values ( null, :api, :name, :minMembers, :maxMembers, :startTime, :finishTime, :finalTime, :penalty, :comboOfTwo, :comboOfThree, :comboOfFourAndMore, :lock )" ));
    query.bindValue( ":api", KetoEvent::API );
    query.bindValue( ":name", eventTitle );
    query.bindValue( ":minMembers", KetoEvent::defaultMinMembers );
    query.bindValue( ":maxMembers", KetoEvent::defaultMaxMembers );
    query.bindValue( ":startTime", KetoEvent::defaultStartTime );
    query.bindValue( ":finishTime", KetoEvent::defaultFinishTime );
    query.bindValue( ":finalTime", KetoEvent::defaultFinalTime );
    query.bindValue( ":penalty", KetoEvent::defaultPenaltyPoints );
    query.bindValue( ":comboOfTwo", KetoEvent::defaultComboOfTwo );
    query.bindValue( ":comboOfThree", KetoEvent::defaultComboOfThree );
    query.bindValue( ":comboOfFourAndMore", KetoEvent::defaultComboOfFourAndMore );
    query.bindValue( ":lock", 0 );

    if ( !query.exec()) {
        Common::error( CLFatalError, QObject::tr( "could not add event, reason - \"%1\"\n" ).arg( query.lastError().text()));
        return;
    }

    // select the new entry
    query.exec( QString( "select * from events where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last sql entry and construct internal entry
    while ( query.next()) {
        Main::instance()->eventList << new Event( query.record(), "events" );
        break;
    }
}

/**
 * @brief Event::loadEvents loads events from database
 * @return success
 */
bool Event::loadEvents() {
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "loading events from database\n" ), Common::EventDebug );

    // read all event entries
    query.exec( "select * from events" );

    // store entries
    while ( query.next()) {
        Event *eventPtr = new Event( query.record(), "events" );
        Main::instance()->eventList << eventPtr;

        // failsafe - api check
        if ( static_cast<unsigned int>( Main::instance()->eventList.last()->api()) < KetoEvent::MinimumAPI ) {
            Common::error( CLSoftError,
                     QObject::tr( "incompatible API - '%1', minimum supported '%2'\n" )
                     .arg( Main::instance()->eventList.last()->api())
                     .arg( KetoEvent::MinimumAPI ));
            Main::instance()->eventList.removeLast();

            // rename database
            Database::unload();
            QFile::rename( Variable::instance()->string( "databasePath" ), QString( "%1_badAPI_%2.db" ).arg( Variable::instance()->string( "databasePath" ).remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
            Database::makePath( Variable::instance()->string( "databasePath", true ));
            Database::load();
            return false;
        }
    }

    // no event entry? - create one
    if ( Main::instance()->eventList.isEmpty())
        Event::add();

    // still nothing?
    if ( Main::instance()->eventList.isEmpty()) {
        Common::error( CLFatalError, QObject::tr( "could not create event\n" ));
    }

    // fixes crash on empty database
    Event *eventPtr = Event::forId( Variable::instance()->integer( "currentEvent" ));
    if ( eventPtr == nullptr )
        if ( Main::instance()->eventList.isEmpty()) {
            Common::error( CLFatalError, QObject::tr( "no valid events and/or corrupted database\n" ));
            return false;
        }

        eventPtr = Main::instance()->eventList.first();

    if ( !Event::setActive( eventPtr ))
        Event::setActive( Main::instance()->eventList.first());

    return true;
}

/**
 * @brief Event::active returns currently active event
 * @return current event entry
 */
Event *Event::active() {
    if ( Main::instance()->activeEvent == nullptr )
        Common::error( CLFatalError, QObject::tr( "no valid events\n" ));

    return Main::instance()->activeEvent;
}

/**
 * @brief Event::setActive sets active event
 * @param eventPtr event entry
 * @return success
 */
bool Event::setActive( Event *eventPtr ) {
    // announce
    Common::print( CLMsg + QObject::tr( "setting '%1' as active event\n" ).arg( eventPtr->name()), Common::EventDebug );

    foreach ( Event *entry, Main::instance()->eventList ) {
        if ( entry == eventPtr ) {
            Main::instance()->activeEvent = entry;
            Variable::instance()->setValue( "currentEvent", eventPtr->id());
            return true;
        }
    }
    return false;
}

/**
 * @brief Event::forId returns event entry for given id
 * @param id event id
 * @return event entry
 */
Event *Event::forId( int id ) {
    foreach ( Event *eventPtr, Main::instance()->eventList ) {
        if ( eventPtr->id() == id )
            return eventPtr;
    }

    return nullptr;
}

/**
 * @brief Event::buildTTList builds event task and team list
 */
void Event::buildTTList() {
    // announce
    Common::print( CLMsg + QObject::tr( "building event TTList\n" ), Common::EventDebug );

    foreach ( Event *eventPtr, Main::instance()->eventList ) {
        eventPtr->teamList.clear();
        eventPtr->taskList.clear();

        foreach ( Team *teamPtr, Main::instance()->teamList ) {
            if ( teamPtr->eventId() == eventPtr->id())
                eventPtr->teamList << teamPtr;
        }

        foreach ( Task *taskPtr, Main::instance()->taskList ) {
            if ( taskPtr->eventId() == eventPtr->id())
                eventPtr->taskList << taskPtr;
        }
    }
}
