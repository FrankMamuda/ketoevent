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
    if ( this->members() < m.currentEvent()->minMembers())
        this->setMembers( m.currentEvent()->minMembers());

    // failsafe members (max)
    if ( this->members() > m.currentEvent()->maxMembers())
        this->setMembers( m.currentEvent()->maxMembers());

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
    Main::stats_t stats = m.getComboStats( this->id());

    if ( this->disqualified())
        return 0;

    foreach ( LogEntry *logPtr, this->logList )
        points += logPtr->points();

    return points + stats.points;
}

/*
================
combos
================
*/
int TeamEntry::combos() const {
    Main::stats_t stats = m.getComboStats( this->id());
    //m.print( QString( "%1 %2\n" ).arg( this->name()).arg( stats.points ));
    return stats.combos;
}

/*
================
total
================
*/
int TeamEntry::total() const {
    Main::stats_t stats = m.getComboStats( this->id());
    return stats.total;
}

/*
================
timeOnTrack
================
*/
int TeamEntry::timeOnTrack() const {
    return m.currentEvent()->startTime().secsTo( this->finishTime()) / 60;
}

/*
================
penalty
================
*/
int TeamEntry::penalty() const {
    int overTime = m.currentEvent()->finishTime().secsTo( this->finishTime()) / 60 + 1;
    if ( overTime > 0 )
        return overTime * m.currentEvent()->penalty();

    return 0;
}

/*
================
disqualified
================
*/
bool TeamEntry::disqualified() const {
    if (( m.currentEvent()->finalTime().secsTo( this->finishTime()) / 60 + 1 ) > 0 )
        return true;

    return false;
}
