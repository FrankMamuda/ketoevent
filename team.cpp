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
void Main::addTeam( const QString &teamName, int members, QTime finishTime, const QString &reviewerName, bool lockState ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "adding new team '%1' with %2 members, reviewed by '%3'\n" ).arg( teamName ).arg( members ).arg( reviewerName ), Main::Team );

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
        this->base.teamList << new TeamEntry( query.record(), "teams" );

    // add to event
    this->currentEvent()->teamList << this->base.teamList.last();
}

/*
================
removeTeam
================
*/
void Main::removeTeam( const QString &teamName ) {
    TeamEntry *teamPtr = NULL;
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "removing team '%1'\n" ).arg( teamName ), Main::Team );

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

/*
================
loadTeams
================
*/
void Main::loadTeams( bool import, bool store ) {
    QSqlQuery query;

    // announce
    m.print( StrMsg + this->tr( "loading teams form database\n" ), Main::Team );

    // read stuff
    if ( import )
        query.exec( "select * from merge.teams" );
    else
        query.exec( "select * from teams" );

    // store entries
    while ( query.next()) {
        TeamEntry *teamPtr = new TeamEntry( query.record(), "teams" );

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
        foreach ( TeamEntry *importedTeamPtr, this->import.teamList ) {
            duplicate = false;

            foreach ( TeamEntry *teamPtr, this->base.teamList ) {
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

/*
================
teamForId
================
*/
TeamEntry *Main::teamForId( int id, bool import ) {
    QList <TeamEntry*> teamList;

    if ( import )
        teamList = m.import.teamList;
    else
        teamList = m.base.teamList;

    foreach ( TeamEntry *teamPtr, teamList ) {
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
    foreach ( TeamEntry *teamPtr, this->base.teamList ) {
        if ( !QString::compare( name, teamPtr->name()))
            return teamPtr;
    }
    return NULL;
}
