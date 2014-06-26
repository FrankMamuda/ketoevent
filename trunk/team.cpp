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
void Main::addTeam(const QString &teamName, int members, QTime finishTime, bool lockState ) {
    QSqlQuery query;

    // avoid duplicates
    if ( this->teamForName( teamName ) != NULL )
        return;

    // perform database update and select last row
    if ( !query.exec( QString( "insert into teams values ( null, '%1', %2, '%3', '%4', null, %5 )" )
                      .arg( teamName )
                      .arg( members )
                      .arg( finishTime.toString( "hh:mm" ))
                      .arg( static_cast<int>( lockState ))
                      /* reserved for reviewerId */
                      .arg( m.currentEvent()->id())
                      )) {
        this->error( StrSoftError + QString( "could not add team, reason: \"%1\"\n" ).arg( query.lastError().text()));
    }
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
}

/*
================
loadTeams
================
*/
void Main::loadTeams( bool import ) {
    QSqlQuery query;

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
        // check for duplicates
        foreach ( TeamEntry *importedTeamPtr, this->import.teamList ) {
            foreach ( TeamEntry *teamPtr, this->base.teamList ) {
                // there's a match
                if ( !QString::compare( teamPtr->name(), importedTeamPtr->name())) {
                    // first time import, just append imported
                    if ( !importedTeamPtr->name().endsWith( " (imported)")) {
                        importedTeamPtr->setName( importedTeamPtr->name() + " (imported)" );
                    }
                    // second time import is a no-go
                    else {
                        m.error( StrSoftError + this->tr( "aborting double import of team \"%1\"\n" ).arg( importedTeamPtr->name()));
                        this->import.teamList.removeOne( teamPtr );
                        continue;
                    }
                }
            }

            // store the new-found team
            importedTeamPtr->store();

            // find a reviewer for the team
            bool duplicate = false;
            foreach ( ReviewerEntry *importedReviewerPtr, this->import.reviewerList ) {
                // found a match
                if ( importedReviewerPtr->id() == importedTeamPtr->reviewerId()) {
                    // check for duplicates
                    foreach ( ReviewerEntry *reviewerPtr, this->base.reviewerList ) {
                        // there's a match
                        if ( !QString::compare( reviewerPtr->name(), importedReviewerPtr->name())) {
                            // first time import, just append imported
                            if ( !importedReviewerPtr->name().endsWith( " (imported)")) {
                                importedReviewerPtr->setName( importedReviewerPtr->name() + " (imported)" );
                            }
                            // second time import is a no-go
                            else {
                                duplicate = true;
                                m.error( StrSoftError + this->tr( "aborting double import of reviewer \"%1\"\n" ).arg( importedReviewerPtr->name()));
                                this->import.reviewerList.removeOne( importedReviewerPtr );
                                break;
                            }
                        }
                    }

                    // if duplicate test failed, set to default reviewer
                    // otherwise, store the new one
                    if ( duplicate )
                        importedTeamPtr->setReviewerId( -1 );
                    else
                        importedReviewerPtr->store();

                    // nothing to do here, break out!
                    break;
                }
            }
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

/*
================
teamComboPoints

  FIXME: ultra-dumb code
================
*/
Main::stats_t Main::getComboStats( int id ) const {
    TeamEntry *teamPtr;
    stats_t stats;

    // reset
    memset( &stats, 0, sizeof( stats ));

    // get team
    teamPtr = m.teamForId( id );
    if ( teamPtr == NULL )
        return stats;

    QList<QPair<int, int> > comboList;
    QPair <int, int> combo;

    // compile combo list
    foreach ( LogEntry *logPtr, teamPtr->logList ) {
        int id = logPtr->comboId();

        if ( id != -1 ) {
            stats.total++;
            bool found = false;
            foreach ( combo, comboList ) {
                if ( combo.first == id ) {
                    comboList << QPair<int, int>( id, combo.second + 1 );
                    comboList.removeOne( combo );
                    found = true;
                }
            }
            if ( !found )
                comboList << QPair<int, int>( id, 1 );
        }
    }

    // get points and combo counts
    stats.combos = comboList.count();
    foreach ( combo, comboList ) {
        if ( combo.second == 2 )
            stats.points += m.currentEvent()->comboOfTwo();
        else if ( combo.second == 3 )
            stats.points += m.currentEvent()->comboOfThree();
        else if ( combo.second >= 4 )
            stats.points += m.currentEvent()->comboOfFourAndMore();
    }
    return stats;
}
