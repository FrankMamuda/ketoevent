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
#include "teamlistmodel.h"
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
    qDebug() << QObject::tr( "adding a new event - '%1'" ).arg( title );

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

    if ( !query.exec())
        qFatal( QObject::tr( "could not add event, reason - \"%1\"" ).arg( query.lastError().text()).toUtf8().constData());

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
    qDebug() << QObject::tr( "loading events from database" );

    // read all event entries
    query.exec( "select * from events" );

    // store entries
    while ( query.next()) {
        Event *event = new Event( query.record(), "events" );
        Main::instance()->eventList << event;

        // failsafe - api check
        if ( static_cast<unsigned int>( Main::instance()->eventList.last()->api()) < KetoEvent::MinimumAPI ) {
            qCritical() <<
                     QObject::tr( "incompatible API - '%1', minimum supported '%2'" )
                     .arg( Main::instance()->eventList.last()->api())
                     .arg( KetoEvent::MinimumAPI );
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
        qFatal( QObject::tr( "could not create event" ).toUtf8().constData());
    }

    // fixes crash on empty database
    Event *event = Event::forId( Variable::instance()->integer( "currentEvent" ));
    if ( event == nullptr && Main::instance()->eventList.isEmpty()) {
        if ( Main::instance()->eventList.isEmpty())
            qFatal( QObject::tr( "no valid events and/or corrupted database" ).toUtf8().constData());
    }
    event = Main::instance()->eventList.first();
    if ( !EventManager::instance()->setActive( event ))
        EventManager::instance()->setActive( Main::instance()->eventList.first());

    return true;
}

/**
 * @brief Event::active returns currently active event
 * @return current event entry
 */
Event *EventManager::active() {
    if ( this->activeEvent == nullptr )
        qFatal( QObject::tr( "no valid events" ).toUtf8().constData());

    return this->activeEvent;
}

/**
 * @brief Event::setActive sets active event
 * @param event event entry
 * @return success
 */
bool EventManager::setActive( Event *event ) {
    if ( event == nullptr )
        return false;

    // announce
    qDebug() << QObject::tr( "setting '%1' as active event" ).arg( event->name());

    foreach ( Event *entry, Main::instance()->eventList ) {
        if ( entry == event ) {
            this->activeEvent = entry;
            Variable::instance()->setValue( "currentEvent", event->id());
            Event::buildTTList();
            emit this->activeEventChanged();
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
    foreach ( Event *event, Main::instance()->eventList ) {
        if ( event->id() == id )
            return event;
    }

    return nullptr;
}

/**
 * @brief Event::buildTTList builds event task and team list
 */
void Event::buildTTList() {
    // announce
    qDebug() << QObject::tr( "building event TTList" );

    // reset model
    Main::instance()->teamModel->beginReset();

    // build task and team list for the event
    foreach ( Event *event, Main::instance()->eventList ) {
        event->teamList.clear();
        event->taskList.clear();

        foreach ( Team *team, Main::instance()->teamList ) {
            if ( team->eventId() == event->id())
                event->teamList << team;
        }

        foreach ( Task *task, Main::instance()->taskList ) {
            if ( task->eventId() == event->id())
                event->taskList << task;
        }
    }

    // reset model
    Main::instance()->teamModel->endReset();
}

/**
 * @brief EventVariable::value
 * @return
 */
EventVariable::EventVariable( const QString &key, const QVariant &defaultValue, Var::Flags flags ) : Var( key, defaultValue, flags ) {
    Main::instance()->connect( EventManager::instance(), &EventManager::activeEventChanged, [ key ]() { Variable::instance()->update( key ); } );
}

/**
 * @brief EventVariable::value
 * @return
 */
QVariant EventVariable::value() const {
    if ( EventManager::instance()->active() != nullptr )
        return EventManager::instance()->active()->record().value( this->key());

    return QVariant();
}

/**
 * @brief EventVariable::setValue
 * @param value
 */
void EventVariable::setValue( const QVariant &value ) {    
    if ( EventManager::instance()->active() != nullptr )
        return EventManager::instance()->active()->setValue( this->key(), value );
}
