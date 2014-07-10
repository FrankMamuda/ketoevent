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
#include <QSqlQuery>

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
        m.base.logList.removeOne( logPtr );
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

    return points + this->bonus();
}

/*
================
addComboPoints
================
*/
void TeamEntry::addComboPoints( int &counter ) {
    if ( counter >= 2 )
        this->m_combos++;

    if ( counter == 2 )
        this->m_bonus += m.currentEvent()->comboOfTwo();
    else if ( counter == 3 )
        this->m_bonus += m.currentEvent()->comboOfThree();
    else if ( counter >= 4 )
        this->m_bonus += m.currentEvent()->comboOfFourAndMore();

    counter = 0;
}

/*
================
comboPoints
================
*/
void TeamEntry::calculateCombos() {
    QSqlQuery query;
    int id, lastId, counter = 0;

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
