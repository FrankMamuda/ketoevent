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

#ifndef TEAM_H
#define TEAM_H

//
// includes
//
#include "database.h"
#include "log.h"
#include <QTime>

class Log;

/**
 * @brief The Team class
 */
class Team : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int members READ members WRITE setMembers )
    Q_PROPERTY( QTime finishTime READ finishTime WRITE setFinishTime )
    Q_PROPERTY( int points READ points )
    Q_PROPERTY( int combos READ combos )
    Q_PROPERTY( int total READ total )
    Q_PROPERTY( int penalty READ penalty )
    Q_PROPERTY( int timeOnTrack READ timeOnTrack )
    Q_PROPERTY( QString reviewer READ reviewer WRITE setReviewer )
    Q_PROPERTY( int eventId READ eventId WRITE setEventId )
    Q_PROPERTY( bool combosCalculated READ combosCalculated WRITE setCombosCalculated )
    Q_PROPERTY( bool locked READ isLocked WRITE setLocked )
    Q_CLASSINFO( "description", "Team SQL Entry" )

public:
    explicit Team( const QSqlRecord &record, const QString &table );
    ~Team();
    QList <Log*> logList;

    // for rankings
    QString name() const { return this->record().value( "name" ).toString(); }
    int members() const { return this->record().value( "members" ).toInt(); }
    QTime finishTime() const { return QTime::fromString( this->record().value( "finishTime" ).toString(), "hh:mm" ); }
    QString reviewer() const { return this->record().value( "reviewer" ).toString(); }
    int eventId() const { return this->record().value( "eventId" ).toInt(); }
    bool isLocked() const { return this->record().value( "lock" ).toBool(); }
    bool isUnlocked() const { return !this->isLocked(); }
    int points() const;
    void calculateCombos();
    int total() const { return this->m_total; }
    int bonus() const { return this->m_bonus; }
    int combos() const { return this->m_combos; }
    int penalty() const;
    float grade() const;
    int timeOnTrack() const;
    bool disqualified() const;
    bool combosCalculated() { return this->m_calculated; }

    // static functions
    static Team *forId( int id );
    static void add( const QString &teamName, int members, QTime finishTime, const QString &reviewerName = QString::null, bool lockState = false );
    static Team *forName( const QString &name, bool currentEvent = false );
    static void remove( const QString &teamName );
    static void loadTeams();

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setMembers( int members ) { this->setValue( "members", members ); }
    void setFinishTime( const QTime &time ) { this->setValue( "finishTime", time.toString( "hh:mm" )); }
    void setReviewer( const QString &reviewer ) { this->setValue( "reviewer", reviewer ); }
    void setEventId( int id ) { this->setValue( "eventId", id ); }
    void lock( bool lock = true ) { this->setValue( "lock", static_cast<int>( lock )); }
    void unlock( bool lock = true ) { this->setValue( "lock", static_cast<int>( !lock )); }
    void addComboPoints( int &counter );
    void setCombosCalculated( bool calculated = true ) { this->m_calculated = calculated; }

private:
    int m_combos;
    int m_total;
    int m_bonus;
    bool m_calculated;
};

#endif // TEAMENTRY_H