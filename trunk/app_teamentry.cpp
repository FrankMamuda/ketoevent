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
#include "app_teamentry.h"
#include "sys_cvarfunc.h"
#include "sys_filesystem.h"
#include "app_main.h"

//
// externals
//
extern pCvar *k_minimumTeamMembers;
extern pCvar *k_maximumTeamMembers;
extern pCvar *k_eventStartTime;
extern pCvar *k_eventFinishTime;
extern pCvar *k_eventFinalTime;
extern pCvar *k_penaltyMultiplier;

/*
================
construct
================
*/
App_TeamEntry::App_TeamEntry( const QString &name, int members, const QTime &startTime, const QTime &finishTime ) {
    // initialize values
    this->setName( name );
    this->setMembers( members );
    this->setStartTime( startTime );
    this->setFinishTime( finishTime );
    this->setDisabled( false );
}

/*
================
setStartTime
================
*/
void App_TeamEntry::setStartTime( const QTime &time ) {
    this->m_startTime = time;

    if ( com.minutesFromQTime( this->m_startTime ) < com.minutesFromQTime( k_eventStartTime->time()))
        this->m_startTime = k_eventStartTime->time();
}

/*
================
setFinishTime
================
*/
void App_TeamEntry::setFinishTime( const QTime &time ) {
    this->m_finishTime = time;

    if ( com.minutesFromQTime( this->m_finishTime ) < com.minutesFromQTime( this->m_startTime ))
        this->m_finishTime = m_startTime;
}

/*
================
setMembers
================
*/
void App_TeamEntry::setMembers( int members ) {
    if ( members < k_minimumTeamMembers->integer())
        members = k_minimumTeamMembers->integer();

    if ( members > k_maximumTeamMembers->integer())
        members = k_maximumTeamMembers->integer();

    this->m_members = members;
}

/*
================
generateXMLTag
================
*/
QString App_TeamEntry::generateXMLTag() {
    QString safeName;

    // make sure buffer is xml safe
    safeName = this->name();
    safeName.replace( "&", "&amp;" );
    safeName.replace( "'", "&apos;" );
    safeName.replace( "\"", "&quot;" );
    safeName.replace( "<", "&lt;"  );
    safeName.replace( ">", "&gt;" );

    // we don't really care about start time since it is the same for everyone
    return QString( "<team name=\"%1\" hour=\"%2\" minute=\"%3\" members=\"%4\" disabled=\"%5\">\n" )
            .arg( safeName )
            .arg( this->finishTime().hour())
            .arg( this->finishTime().minute())
            .arg( this->members())
            .arg( this->disabled());
}

/*
================
generateXMLLog
================
*/
QString App_TeamEntry::generateXMLLog() {
    QString buffer;

    // generate buffer
    buffer = this->generateXMLTag();
    foreach ( App_TaskLog *logPtr, this->logList )
        buffer.append( logPtr->generateXMLTag());
    buffer.append( "</team>\n" );

    return buffer;
}

/*
================
save
================
*/
void App_TeamEntry::save() {
    QString buffer;
    fileHandle_t out;

    // this really should not happen
    if ( !this->filenameSet()) {
        com.error( StrSoftError + this->tr( "cannot save team \"%1\" log, no filename set\n" ).arg( this->name()));
        return;
    }

    // generate buffer
    buffer = this->generateXMLLog();

    // open file in read mode
    fs.open( pFile::Write, this->filename(), out );
    fs.print( out, buffer );
    fs.close( out );
}

/*
================
points
================
*/
int App_TeamEntry::points() const {
    int points = 0;
    int finalTime = com.minutesFromQTime( k_eventFinalTime->time());
    int timeOnTrack = this->timeOnTrack();
    int startTime = com.minutesFromQTime( k_eventStartTime->time());

    if ( startTime + timeOnTrack > finalTime )
        return 0;

    foreach ( App_TaskLog *logPtr, this->logList )
        points += logPtr->calculate();

    return points;
}

/*
================
challenges
================
*/
int App_TeamEntry::challenges() const {
    int challenges = 0;

    foreach ( App_TaskLog *logPtr, this->logList )
        challenges += static_cast<int>( logPtr->task()->isChallenge());

    return challenges;
}

/*
================
combos
================
*/
int App_TeamEntry::combos() const {
    int combos = 0;

    foreach ( App_TaskLog *logPtr, this->logList )
        combos += static_cast<bool>( logPtr->combo());

    return combos;
}

/*
================
grade
================
*/
float App_TeamEntry::grade() const {
    float totalGrade = 0.0f;
    int numSubjects = 0;

    foreach ( App_TaskEntry *entryPtr, m.taskList ) {
        if ( entryPtr->type() == App_TaskEntry::Special )
            numSubjects++;
    }

    if ( !numSubjects )
        return 0.0f;

    foreach ( App_TaskLog *logPtr, this->logList ) {
        if ( logPtr->task()->type() == App_TaskEntry::Special )
            totalGrade += logPtr->calculate();
    }
    // round it
    return floorf(( totalGrade / static_cast<float>( numSubjects )) * 100 + 0.5 ) / 100;
}

/*
================
timeOnTrack
================
*/
int App_TeamEntry::timeOnTrack() const {
    return com.minutesFromQTime( this->finishTime()) - com.minutesFromQTime( k_eventStartTime->time()) + 1;
}

/*
================
penalty
================
*/
int App_TeamEntry::penalty() const {
    int startTime = com.minutesFromQTime( k_eventStartTime->time());
    int finishTime = com.minutesFromQTime( k_eventFinishTime->time());
    int timeOnTrack = this->timeOnTrack();

    if ( startTime + timeOnTrack > finishTime )
        return (( startTime + timeOnTrack ) - finishTime ) * k_penaltyMultiplier->value();
    else
        return 0;
}

