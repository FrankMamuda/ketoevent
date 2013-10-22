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

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
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
    int points = 0;

    if ( this->disqualified())
        return 0;

    foreach ( LogEntry *logPtr, this->logList )
        points += logPtr->points();

    return points;
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
timeOnTrack
================
*/
int TeamEntry::timeOnTrack() const {
    return m.var( "time/start" )->time().secsTo( this->finishTime()) / 60;
}

/*
================
penalty
================
*/
int TeamEntry::penalty() const {
    int overTime = m.var( "time/finish" )->time().secsTo( this->finishTime()) / 60 + 1;
    if ( overTime > 0 )
        return overTime * m.var( "penaltyMultiplier" )->integer();

    return 0;
}

/*
================
disqualified
================
*/
bool TeamEntry::disqualified() const {
    if (( m.var( "time/final" )->time().secsTo( this->finishTime()) / 60 + 1 ) > 0 )
        return true;

    return false;
}
