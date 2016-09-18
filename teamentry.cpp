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
// includes
//
#include "teamentry.h"
#include "main.h"
#include <QSqlQuery>

/**
 * @brief Team::Team
 * @param record
 * @param table
 */
Team::Team( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );

    // no calculation has been peformed yet
    this->setCombosCalculated( false );

    // failsafe members (min)
    if ( this->members() < m.currentEvent()->minMembers())
        this->setMembers( m.currentEvent()->minMembers());

    // failsafe members (max)
    if ( this->members() > m.currentEvent()->maxMembers())
        this->setMembers( m.currentEvent()->maxMembers());

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/**
 * @brief Team::~Team
 */
Team::~Team() {
    foreach ( Log *logPtr, this->logList ) {
        m.base.logList.removeOne( logPtr );
        delete logPtr;
    }
    this->logList.clear();
}

/**
 * @brief Team::points
 * @return
 */
int Team::points() const {
    int points = 0;

    if ( this->disqualified())
        return 0;

    foreach ( Log *logPtr, this->logList )
        points += logPtr->points();

    return points + this->bonus();
}

/**
 * @brief Team::addComboPoints
 * @param counter
 */
void Team::addComboPoints( int &counter ) {
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

/**
 * @brief Team::calculateCombos
 */
void Team::calculateCombos() {
    QSqlQuery query;
    int id, lastId = -1, counter = 0;

    // abort if not needed
    if ( this->combosCalculated())
        return;

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
   // m.print( QString( "  total %1\n" ).arg( this->m_total ));

    this->addComboPoints( counter );
}

/**
 * @brief Team::timeOnTrack
 * @return
 */
int Team::timeOnTrack() const {
    return m.currentEvent()->startTime().secsTo( this->finishTime()) / 60;
}

/**
 * @brief Team::penalty
 * @return
 */
int Team::penalty() const {
    int overTime = m.currentEvent()->finishTime().secsTo( this->finishTime()) / 60 + 1;
    if ( overTime > 0 )
        return overTime * m.currentEvent()->penalty();

    return 0;
}

/**
 * @brief Team::disqualified
 * @return
 */
bool Team::disqualified() const {
    if (( m.currentEvent()->finalTime().secsTo( this->finishTime()) / 60 + 1 ) > 0 )
        return true;

    return false;
}
