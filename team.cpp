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
#include "team.h"
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Team::Team
 * @param record
 * @param table
 */
Team::Team( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );

    // no calculation has been peformed yet
    this->setCombosCalculated( false );

    // failsafe members (min)
    if ( this->members() < Event::active()->minMembers())
        this->setMembers( Event::active()->minMembers());

    // failsafe members (max)
    if ( this->members() > Event::active()->maxMembers())
        this->setMembers( Event::active()->maxMembers());

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/**
 * @brief Team::~Team
 */
Team::~Team() {
    foreach ( Log *logPtr, this->logList ) {
        m.logList.removeOne( logPtr );
        delete logPtr;
    }
    this->logList.clear();
}

/**
 * @brief Team::points
 * @return
 */
int Team::points() const {
    int points = 0;

    if ( this->disqualified())
        return 0;

    foreach ( Log *logPtr, this->logList )
        points += logPtr->points();

    return points + this->bonus();
}

/**
 * @brief Team::addComboPoints
 * @param counter
 */
void Team::addComboPoints( int &counter ) {
    if ( counter >= 2 )
        this->m_combos++;

    if ( counter == 2 )
        this->m_bonus += Event::active()->comboOfTwo();
    else if ( counter == 3 )
        this->m_bonus += Event::active()->comboOfThree();
    else if ( counter >= 4 )
        this->m_bonus += Event::active()->comboOfFourAndMore();

    counter = 0;
}

/**
 * @brief Team::calculateCombos
 */
void Team::calculateCombos() {
    QSqlQuery query;
    int id, lastId = -1, counter = 0;

    // abort if not needed
    if ( this->combosCalculated())
        return;

    // reset stats
    this->m_combos = 0;
    this->m_bonus = 0;
    this->m_total = 0;

    // get combos for the team
    query.exec( QString( "select * from logs where teamId=%1 and comboId!=-1 and value>0 order by comboId asc" ).arg( this->id()));

    // go through list
    while ( query.next()) {
        id = query.record().value( "comboId" ).toInt();

        if ( this->total() == 0 )
            lastId = id;

        if ( id != lastId ) {
            lastId = id;
            this->addComboPoints( counter );
        }
        counter++;
        this->m_total++;
    }

    this->addComboPoints( counter );
}

/**
 * @brief Team::timeOnTrack
 * @return
 */
int Team::timeOnTrack() const {
    return Event::active()->startTime().secsTo( this->finishTime()) / 60;
}

/**
 * @brief Team::penalty
 * @return
 */
int Team::penalty() const {
    int overTime = Event::active()->finishTime().secsTo( this->finishTime()) / 60 + 1;
    if ( overTime > 0 )
        return overTime * Event::active()->penalty();

    return 0;
}

/**
 * @brief Team::disqualified
 * @return
 */
bool Team::disqualified() const {
    if (( Event::active()->finalTime().secsTo( this->finishTime()) / 60 + 1 ) > 0 )
        return true;

    return false;
}

/**
 * @brief Team::forId
 * @return
 */
Team *Team::forId( int id ) {
    foreach ( Team *teamPtr, m.teamList ) {
        if ( teamPtr->id() == id )
            return teamPtr;
    }
    return NULL;
}

/**
 * @brief Team::add
 * @param teamName
 * @param members
 * @param finishTime
 * @param reviewerName
 * @param lockState
 */
void Team::add( const QString &teamName, int members, QTime finishTime, const QString &reviewerName, bool lockState ) {
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "adding new team '%1' with %2 members, reviewed by '%3'\n" ).arg( teamName ).arg( members ).arg( reviewerName ), Common::TeamDebug );

    // avoid duplicates
    if ( Team::forName( teamName, true ) != NULL )
        return;

    // perform database update and select last row
    // NOTE: this is sensitive to API changes, cannot avoid hardcoding unless we use addTeam( QVariantList ), which is also not ideal
    query.prepare( "insert into teams values ( null, :name, :members, :finishTime, :lock, :reviewer, :eventId )" );
    query.bindValue( ":name", teamName );
    query.bindValue( ":members", members );
    query.bindValue( ":finishTime", finishTime.toString( "hh:mm" ));
    query.bindValue( ":lock",  static_cast<int>( lockState ));
    query.bindValue( ":reviewer", reviewerName );
    query.bindValue( ":eventId", Event::active()->id());

    if ( !query.exec()) {
        Common::error( CLSoftError, QObject::tr( "could not add team, reason: \"%1\"\n" ).arg( query.lastError().text()));
        return;
    }

    // select the new entry
    query.exec( QString( "select * from teams where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last entry and construct internal entry
    while ( query.next())
        m.teamList << new Team( query.record(), "teams" );

    // add to event
    Event::active()->teamList << m.teamList.last();
}

/**
 * @brief Team::remove
 * @param teamName
 */
void Team::remove( const QString &teamName ) {
    Team *teamPtr = NULL;
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "removing team '%1'\n" ).arg( teamName ), Common::TeamDebug );

    // find team
    teamPtr = Team::forName( teamName, true );

    // failsafe
    if ( teamPtr == NULL )
        return;

    // remove team and logs from db
    query.exec( QString( "delete from teams where id=%1" ).arg( teamPtr->id()));
    query.exec( QString( "delete from logs where teamId=%1" ).arg( teamPtr->id()));

    // remove from display
    m.teamList.removeAll( teamPtr );
    Event::active()->teamList.removeAll( teamPtr );
}

/**
 * @brief Team::loadTeams
 * @param import
 * @param store
 */
void Team::loadTeams() {
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "loading teams form database\n" ), Common::TeamDebug );

    // read all team entries
    query.exec( "select * from teams" );

    // store entries
    while ( query.next())
        m.teamList << new Team( query.record(), "teams" );

    // sort alphabetically
    m.sort( Main::Teams );
}

/**
 * @brief Team::teamForName
 * @param name
 * @param currentEvent
 * @return
 */
Team *Team::forName( const QString &name, bool currentEvent ) {
    QList <Team*> teamList;

    if ( currentEvent )
        teamList = Event::active()->teamList;
    else
        teamList = m.teamList;

    foreach ( Team *teamPtr, teamList ) {
        if ( !QString::compare( name, teamPtr->name()))
            return teamPtr;
    }
    return NULL;
}
