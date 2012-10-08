/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
// includes
//
#include "app_logparser.h"
#include "app_teamentry.h"
#include "app_main.h"
#include "sys_filesystem.h"
#include <QMessageBox>
#include <QIcon>

//
// externals
//
extern pCvar *k_eventStartTime;

/*
================
startElement
================
*/
bool App_LogParser::startElement( const QString &, const QString &, const QString &name, const QXmlAttributes &attributes ) {
    int y;

    if ( !QString::compare( name, "teams" )) {
        this->setDataBaseParse();
        return true;
    } else if ( !QString::compare( name, "team" ))
        this->setState( Document );
    else if ( !QString::compare( name, "log" ))
        this->setState( Log );
    else {
        com.error( Sys_Common::SoftError, QObject::trUtf8( "App_LogParser::startElement: unknown element \"%1\"\n" ).arg( name ));
        return false;
    }

    if ( this->state() == Document ) {
        // set defaults
        this->teamData.name = QString::null;
        this->teamData.hour = this->teamData.minute = this->teamData.members = 0;
        this->logData.clear();

        // get attributes
        for ( y = 0; y < attributes.count(); y++ ) {
            if ( !QString::compare( attributes.qName( y ), "name" ))
                this->teamData.name = attributes.value( y );
            else if ( !QString::compare( attributes.qName( y ), "hour" ))
                this->teamData.hour = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "minute" ))
                this->teamData.minute = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "members" ))
                this->teamData.members = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "disabled" )) {
                if ( static_cast<bool>( attributes.value( y ).toInt())) {
                    com.print( QObject::trUtf8( "App_LogParser::startElement: team \"%1\" disabled, abort\n" ).arg( this->teamData.name ));
                    return false;
                }
            }
        }
    } else if ( this->state() == Log ) {
        // begin parsing log entry
        logData_t log;

        // set defaults
        log.hash = QString::null;
        log.value = 0;
        log.combo = App_TaskLog::NoCombo;

        // get attributes
        for ( y = 0; y < attributes.count(); y++ ) {
            if ( !QString::compare( attributes.qName( y ), "hash" ))
                log.hash = attributes.value( y );
            else if ( !QString::compare( attributes.qName( y ), "value" ))
                log.value = attributes.value( y ).toInt();
            else if ( !QString::compare( attributes.qName( y ), "combo" ))
                log.combo = static_cast<App_TaskLog::Combos>( attributes.value( y ).toInt());
        }

        // ignore invalid logs
        if ( !log.hash.isEmpty() && log.value )
            this->logData << log;
    }
    return true;
}

/*
================
endElement
================
*/
bool App_LogParser::endElement( const QString &, const QString &, const QString &name ) {
    if ( !QString::compare( name, "team" )) {
        App_TeamEntry *teamPtr;
        App_TeamEntry *teamDupPtr;
        bool replace = false;

        // generate new team
        teamPtr = new App_TeamEntry( this->teamData.name, this->teamData.members, k_eventStartTime->time(), QTime( this->teamData.hour, this->teamData.minute ));

        // add logs to team data
        foreach ( logData_t log, this->logData ) {
            App_TaskEntry *entryPtr;

            // first get corresponding task entry that matches hash value
            entryPtr = m.taskForHash( log.hash );

            // bogus task? okay, go on anyway
            if ( entryPtr == NULL ) {
                com.print( QObject::trUtf8( "App_LogParser::endElement: unknown task with hash \"%1\"\n" ).arg( log.hash ));
                continue;
            }

            // failsafe combo
            switch ( log.combo ) {
            case App_TaskLog::NoCombo:
            case App_TaskLog::Single:
            case App_TaskLog::Double:
            case App_TaskLog::Triple:
                break;

            default:
                com.error( Sys_Common::SoftError, QObject::trUtf8( "App_LogParser::endElement: unknown combo with value \"%1\"\n" ).arg( static_cast<int>( log.combo )));
                delete teamPtr;
                return false;
            }

            // finally add the new entry
            App_TaskLog *taskPtr = new App_TaskLog( log.value, log.combo, entryPtr );
            teamPtr->logList << taskPtr;
        }

        // failsafe
        teamDupPtr = m.findTeamByName( this->teamData.name );
        if ( teamDupPtr != NULL ) {
            if ( this->isDataBase()) {
                QMessageBox msgBox;
                int state;

                // set up
                msgBox.setText( QObject::trUtf8( "Team \"%1\" already exists with %2 logs and %3 points" ).arg( teamDupPtr->name()).arg( teamDupPtr->logList.count()).arg( teamDupPtr->points()));
                msgBox.setInformativeText( QObject::trUtf8( "Would you like to replace it with %1 logs (%2 points)?" ).arg( teamPtr->logList.count()).arg( teamPtr->points()));
                msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
                msgBox.setDefaultButton( QMessageBox::Yes );
                msgBox.setWindowIcon( QIcon( ":/icons/keto_48" ));
                msgBox.setIcon( QMessageBox::Question );

                // return restult
                state = msgBox.exec();
                switch ( state ) {
                case QMessageBox::Yes:
                    replace = true;
                    break;

                case QMessageBox::No:
                default:
                    return true;
                }
            } else {
                com.error( Sys_Common::SoftError, QObject::trUtf8( "App_LogParser::endElement: duplicate team \"%1\"\n" ).arg( this->teamData.name ));
                delete teamPtr;
                return false;
            }
        }

        // store filename
        if ( this->isDataBase()) {
            if ( replace )
                teamPtr->setFilename( teamDupPtr->filename());
            else
                teamPtr->setFilename( fs.safeTeamFilename( teamPtr->name()));
        } else
            teamPtr->setFilename( this->filename());

        // finally add new team
        if ( !m.addTeam( teamPtr, false, true )) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "App_LogParser::endElement: could not add team \"%1\"\n" ).arg( teamPtr->name()));
            delete teamPtr;
            return false;
        }

        // since we're importing - save the file
        if ( this->isDataBase())
            teamPtr->save();
    } else if ( !QString::compare( name, "log" ) || !QString::compare( name, "teams" )) {
        this->setState( Document );
    } else {
        com.error( Sys_Common::SoftError, QObject::trUtf8( "App_LogParser::endElement: unknown element \"%1\"\n" ).arg( name ));
        return false;
    }
    return true;
}
