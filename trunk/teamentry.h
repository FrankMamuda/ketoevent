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

#ifndef TEAMENTRY_H
#define TEAMENTRY_H

//
// includes
//
#include "databaseentry.h"
#include "logentry.h"
#include <QTime>

//
// class: TeamEntry
//
class TeamEntry : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int members READ members WRITE setMembers )
    Q_PROPERTY( QTime finishTime READ finishTime WRITE setFinishTime )
    Q_PROPERTY( int points READ points )
    Q_PROPERTY( int combos READ combos )
    Q_PROPERTY( int total READ total )
    Q_PROPERTY( int penalty READ penalty )
    Q_PROPERTY( int timeOnTrack READ timeOnTrack )
    Q_PROPERTY( int reviewerId READ reviewerId WRITE setReviewerId )
    Q_PROPERTY( int eventId READ eventId WRITE setEventId )
    Q_CLASSINFO( "description", "Team SQL Entry" )

public:
    explicit TeamEntry( const QSqlRecord &record, const QString &table );
    ~TeamEntry();
    QList <LogEntry*> logList;

    // for rankings
    QString name() const { return this->record().value( "name" ).toString(); }
    int members() const { return this->record().value( "members" ).toInt(); }
    QTime finishTime() const { return QTime::fromString( this->record().value( "finishTime" ).toString(), "hh:mm" ); }
    int reviewerId() const { return this->record().value( "reviewerId" ).toInt(); }
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

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setMembers( int members ) { this->setValue( "members", members ); }
    void setFinishTime( const QTime &time ) { this->setValue( "finishTime", time.toString( "hh:mm" )); }
    void setReviewerId( int id ) { this->setValue( "reviewerId", id ); }
    void setEventId( int id ) { this->setValue( "eventId", id ); }
    void lock( bool lock = true ) { this->setValue( "lock", static_cast<int>( lock )); }
    void unlock( bool lock = true ) { this->setValue( "lock", static_cast<int>( !lock )); }
    void addComboPoints( int &counter );

private:
    int m_combos;
    int m_total;
    int m_bonus;
};

#endif // TEAMENTRY_H