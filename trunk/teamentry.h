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
    Q_PROPERTY( QTime startTime READ startTime WRITE setStartTime )
    Q_PROPERTY( QTime finishTime READ finishTime WRITE setFinishTime )
    Q_PROPERTY( int points READ points )
    Q_PROPERTY( int challenges READ challenges )
    Q_PROPERTY( int combos READ combos )
    Q_PROPERTY( int penalty READ penalty )
    Q_PROPERTY( int grade READ grade )
    Q_PROPERTY( int timeOnTrack READ timeOnTrack )

public:
    explicit TeamEntry( const QSqlRecord &record, const QString &table );
    ~TeamEntry();
    QList <LogEntry*> logList;

    // for rankings
    QString name() const { return this->record().value( "name" ).toString(); }
    int members() const { return this->record().value( "members" ).toInt(); }
    QTime startTime() const { return QTime::fromString( this->record().value( "start" ).toString(), "hh:mm" ); }
    QTime finishTime() const { return QTime::fromString( this->record().value( "finish" ).toString(), "hh:mm" ); }
    int points() const;
    int challenges() const;
    int combos() const;
    int penalty() const;
    float grade() const;
    int timeOnTrack() const;

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setMembers( int members ) { this->setValue( "members", members ); }
    void setStartTime( QTime time ) { this->setValue( "start", time.toString( "hh:mm" )); }
    void setFinishTime( QTime time ) { this->setValue( "finish", time.toString( "hh:mm" )); }
};

#endif // TEAMENTRY_H
