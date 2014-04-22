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
// team.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
addTeam
================
*/
void Main::addTeam( const QString &teamName, int members, QTime finishTime, bool lockState ) {
    QSqlQuery query;

    // avoid duplicates
    if ( m.teamForName( teamName ) != NULL )
        return;

    // perform database update and select last row
    if ( !query.exec( QString( "insert into teams values ( null, '%1', %2, '%3', '%4', null )" )
                      .arg( teamName )
                      .arg( members )
                      .arg( finishTime.toString( "hh:mm" ))
                      .arg( static_cast<int>( lockState ))
                      )) {
        this->error( StrSoftError + QString( "could not add team, reason: \"%1\"\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from teams where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last entry and construct internal entry
    while ( query.next())
        this->teamList << new TeamEntry( query.record(), "teams" );
}

/*
================
removeTeam
================
*/
void Main::removeTeam( const QString &teamName ) {
    TeamEntry *teamPtr = NULL;
    QSqlQuery query;

    // find team
    teamPtr = m.teamForName( teamName );

    // failsafe
    if ( teamPtr == NULL )
        return;

    // remove team and logs from db
    query.exec( QString( "delete from teams where id=%1" ).arg( teamPtr->id()));
    query.exec( QString( "delete from logs where teamId=%1" ).arg( teamPtr->id()));

    // remove from display
    this->teamList.removeAll( teamPtr );
}

/*
================
loadTeams
================
*/
void Main::loadTeams() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from teams" );

    // store entries
    while ( query.next())
        this->teamList << new TeamEntry( query.record(), "teams" );

    // sort alphabetically
    this->sort( Main::Teams );
}

/*
================
teamForId
================
*/
TeamEntry *Main::teamForId( int id ) {
    foreach ( TeamEntry *teamPtr, this->teamList ) {
        if ( teamPtr->id() == id )
            return teamPtr;
    }
    return NULL;
}

/*
================
teamForName
================
*/
TeamEntry *Main::teamForName( const QString &name ) {
    foreach ( TeamEntry *teamPtr, this->teamList ) {
        if ( !QString::compare( name, teamPtr->name()))
            return teamPtr;
    }
    return NULL;
}