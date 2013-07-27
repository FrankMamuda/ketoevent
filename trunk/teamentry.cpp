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
// includes
//
#include "teamentry.h"
#include "main.h"
#include "math.h"

/*
================
construct
================
*/
TeamEntry::TeamEntry( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );

    // failsafe members (min)
    if ( this->members() < m.var( "members/min" )->integer())
        this->setMembers( m.var( "members/min" )->integer());

    // failsafe members (max)
    if ( this->members() > m.var( "members/max" )->integer())
        this->setMembers( m.var( "members/max" )->integer());
}

/*
================
destruct
================
*/
TeamEntry::~TeamEntry() {
    foreach ( LogEntry *logPtr, this->logList ) {
        m.logList.removeOne( logPtr );
        delete logPtr;
    }
    this->logList.clear();
}

/*
================
points
================
*/
int TeamEntry::points() const {
    int points = 0, finalTime, timeOnTrack, startTime;

    finalTime = m.minutes( m.var( "time/final" )->time());
    timeOnTrack = this->timeOnTrack();
    startTime = m.minutes( m.var( "time/start" )->time());

    if ( startTime + timeOnTrack > finalTime )
        return 0;

    foreach ( LogEntry *logPtr, this->logList )
        points += logPtr->points();

    return points;
}

/*
================
challenges
================
*/
int TeamEntry::challenges() const {
    int challenges = 0;

    foreach ( LogEntry *logPtr, this->logList ) {
        TaskEntry *taskPtr = m.taskForId( logPtr->taskId());

        if ( taskPtr != NULL )
            challenges += static_cast<int>( taskPtr->isChallenge());
    }

    return challenges;
}

/*
================
combos
================
*/
int TeamEntry::combos() const {
    int combos = 0;

    foreach ( LogEntry *logPtr, this->logList )
        combos += static_cast<int>( logPtr->combo());

    return combos;
}

/*
================
grade
================
*/
float TeamEntry::grade() const {
    float totalGrade = 0.0f;
    int numSubjects = 0;

    foreach ( TaskEntry *taskPtr, m.taskList ) {
        if ( taskPtr->type() == TaskEntry::Special )
            numSubjects++;
    }

    if ( !numSubjects )
        return 0.0f;

    foreach ( LogEntry *logPtr, this->logList ) {
        TaskEntry *taskPtr = m.taskForId( logPtr->taskId());

        if ( taskPtr == NULL )
            continue;

        if ( taskPtr->type() == TaskEntry::Special )
            totalGrade += logPtr->points();
    }

    // round it
    return floorf(( totalGrade / static_cast<float>( numSubjects )) * 100 + 0.5 ) / 100;
}

/*
================
timeOnTrack
================
*/
int TeamEntry::timeOnTrack() const {
    return m.minutes( this->finishTime()) - m.minutes( m.var( "time/start" )->time()) + 1;
}

/*
================
penalty
================
*/
int TeamEntry::penalty() const {
    int startTime = m.minutes( m.var( "time/start" )->time());
    int finishTime = m.minutes( m.var( "time/finish" )->time());
    int timeOnTrack = this->timeOnTrack();

    if ( startTime + timeOnTrack > finishTime )
        return (( startTime + timeOnTrack ) - finishTime ) * m.var( "penaltyMultiplier" )->integer();
    else
        return 0;
}
