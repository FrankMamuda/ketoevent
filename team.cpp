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
#include "team.h"
#include "main.h"
#include "teamlistmodel.h"
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
    this->connect( this, SIGNAL( changed()), Main::instance(), SLOT( update()));
}

/**
 * @brief Team::~Team
 */
Team::~Team() {
    foreach ( Log *log, this->logList ) {
        Main::instance()->logList.removeOne( log );
        delete log;
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

    foreach ( Log *log, this->logList )
        points += log->points();

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
    foreach ( Team *team, Main::instance()->teamList ) {
        if ( team->id() == id )
            return team;
    }
    return nullptr;
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

    // reset model
    Main::instance()->teamModel->beginReset();

    // announce
    Common::print( CLMsg + QObject::tr( "adding new team '%1' with %2 members, reviewed by '%3'\n" ).arg( teamName ).arg( members ).arg( reviewerName ), Common::TeamDebug );

    // avoid duplicates
    if ( Team::forName( teamName, true ) != nullptr )
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
        Main::instance()->teamList << new Team( query.record(), "teams" );

    // add to event
    Event::active()->teamList << Main::instance()->teamList.last();

    // reset model
    Main::instance()->teamModel->endReset();
}

/**
 * @brief Team::remove
 * @param teamName
 */
void Team::remove( const QString &teamName ) {
    Team *team = nullptr;
    QSqlQuery query;

    // reset model
    Main::instance()->teamModel->beginReset();

    // announce
    Common::print( CLMsg + QObject::tr( "removing team '%1'\n" ).arg( teamName ), Common::TeamDebug );

    // find team
    team = Team::forName( teamName, true );

    // failsafe
    if ( team == nullptr )
        return;

    // remove logs from memory
    foreach ( Log *log, Main::instance()->logList ) {
        if ( log->teamId() == team->id())
            Main::instance()->logList.removeAll( log );
    }

    // remove team and logs from db
    if ( !query.exec( QString( "delete from teams where id=%1" ).arg( team->id())))
        Common::error( CLSoftError, QObject::tr( "could not remove team, reason: \"%1\"\n" ).arg( query.lastError().text()));

    if ( !query.exec( QString( "delete from logs where teamId=%1" ).arg( team->id())))
        Common::error( CLSoftError, QObject::tr( "could not remove team log, reason: \"%1\"\n" ).arg( query.lastError().text()));

    // remove from display
    Main::instance()->teamList.removeAll( team );
    Event::active()->teamList.removeAll( team );

    // reset model
    Main::instance()->teamModel->endReset();
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
        Main::instance()->teamList << new Team( query.record(), "teams" );

    // sort alphabetically
    Main::instance()->sort( Main::Teams );
}

/**
 * @brief Team::teamForName
 * @param name
 * @param currentEvent
 * @return
 */
Team *Team::forName( const QString &name, bool currentEvent ) {
    QList <Team*> teamList;

    teamList = currentEvent ? Event::active()->teamList : Main::instance()->teamList;
    foreach ( Team *team, teamList ) {
        if ( !QString::compare( name, team->name()))
            return team;
    }
    return nullptr;
}
