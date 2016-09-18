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
// team.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/**
 * @brief Main::addTeam
 * @param teamName
 * @param members
 * @param finishTime
 * @param reviewerName
 * @param lockState
 */
void Main::addTeam( const QString &teamName, int members, QTime finishTime, const QString &reviewerName, bool lockState ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "adding new team '%1' with %2 members, reviewed by '%3'\n" ).arg( teamName ).arg( members ).arg( reviewerName ), Main::TeamDebug );

    // avoid duplicates
    if ( this->teamForName( teamName ) != NULL )
        return;

    // perform database update and select last row
#ifdef SQL_PREPARE_STATEMENTS
    query.prepare( "insert into teams values ( null, :name, :members, :finishTime, :lock, :reviewer, :eventId )" );
    query.bindValue( ":name", teamName );
    query.bindValue( ":members", members );
    query.bindValue( ":finishTime", finishTime.toString( "hh:mm" ));
    query.bindValue( ":lock",  static_cast<int>( lockState ));
    query.bindValue( ":reviewer", reviewerName );
    query.bindValue( ":eventId", m.currentEvent()->id());

    if ( !query.exec())
#else
    if ( !query.exec( QString( "insert into teams values ( null, '%1', %2, '%3', '%4', '%5', %6 )" )
                      .arg( teamName )
                      .arg( members )
                      .arg( finishTime.toString( "hh:mm" ))
                      .arg( static_cast<int>( lockState ))
                      .arg( reviewerName )
                      .arg( m.currentEvent()->id())
                      ))
#endif
        this->error( StrSoftError, QString( "could not add team, reason: \"%1\"\n" ).arg( query.lastError().text()));

    // select the new entry
    query.exec( QString( "select * from teams where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last entry and construct internal entry
    while ( query.next())
        this->base.teamList << new Team( query.record(), "teams" );

    // add to event
    this->currentEvent()->teamList << this->base.teamList.last();
}

/**
 * @brief Main::removeTeam
 * @param teamName
 */
void Main::removeTeam( const QString &teamName ) {
    Team *teamPtr = NULL;
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "removing team '%1'\n" ).arg( teamName ), Main::TeamDebug );

    // find team
    teamPtr = this->teamForName( teamName );

    // failsafe
    if ( teamPtr == NULL )
        return;

    // remove team and logs from db
    query.exec( QString( "delete from teams where id=%1" ).arg( teamPtr->id()));
    query.exec( QString( "delete from logs where teamId=%1" ).arg( teamPtr->id()));

    // remove from display
    this->base.teamList.removeAll( teamPtr );
    this->currentEvent()->teamList.removeAll( teamPtr );
}

/**
 * @brief Main::loadTeams
 * @param import
 * @param store
 */
void Main::loadTeams( bool import, bool store ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "loading teams form database\n" ), Main::TeamDebug );

    // read stuff
    if ( import )
        query.exec( "select * from merge.teams" );
    else
        query.exec( "select * from teams" );

    // store entries
    while ( query.next()) {
        Team *teamPtr = new Team( query.record(), "teams" );

        if ( import ) {
            teamPtr->setImported();
            this->import.teamList << teamPtr;
        } else
            this->base.teamList << teamPtr;
    }

    // sort alphabetically
    if ( !import )
        this->sort( Main::Teams );
    else {
        bool duplicate = false;

        // check for duplicates
        foreach ( Team *importedTeamPtr, this->import.teamList ) {
            duplicate = false;

            foreach ( Team *teamPtr, this->base.teamList ) {
                // there's a match
                if ( !QString::compare( teamPtr->name(), importedTeamPtr->name())) {
                    // first time import, just append imported
                    if ( !importedTeamPtr->name().endsWith( " (imported)")) {
                        importedTeamPtr->setName( importedTeamPtr->name() + " (imported)" );
                    }
                    // second time import is a no-go
                    else {
                        duplicate = true;
                        m.error( StrSoftError, this->tr( "aborting double import of team \"%1\"\n" ).arg( importedTeamPtr->name()));
                        this->import.teamList.removeOne( teamPtr );
                        continue;
                    }
                }
            }

            // store the new-found team
            if ( !duplicate && store )
                importedTeamPtr->store();
        }
    }
}

/**
 * @brief Main::teamForId
 * @param id
 * @param import
 * @return
 */
Team *Main::teamForId( int id, bool import ) {
    QList <Team*> teamList;

    if ( import )
        teamList = m.import.teamList;
    else
        teamList = m.base.teamList;

    foreach ( Team *teamPtr, teamList ) {
        if ( teamPtr->id() == id )
            return teamPtr;
    }
    return NULL;
}

/**
 * @brief Main::teamForName
 * @param name
 * @return
 */
Team *Main::teamForName( const QString &name ) {
    foreach ( Team *teamPtr, this->base.teamList ) {
        if ( !QString::compare( name, teamPtr->name()))
            return teamPtr;
    }
    return NULL;
}
