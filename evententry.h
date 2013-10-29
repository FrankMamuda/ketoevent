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

#ifndef EVENTENTRY_H
#define EVENTENTRY_H

//
// includes
//
#include "databaseentry.h"
#include <QTime>

//
// class: EventEntry
//
class EventEntry : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int minMembers READ minMembers WRITE setMinMembers )
    Q_PROPERTY( int maxMembers READ maxMembers WRITE setMaxMembers )
    Q_PROPERTY( int api READ api WRITE setAPIversion )
    Q_PROPERTY( int singleCombo READ singleCombo WRITE setSingleCombo )
    Q_PROPERTY( int doubleCombo READ doubleCombo WRITE setDoubleCombo )
    Q_PROPERTY( int tripleCombo READ tripleCombo WRITE setTripleCombo )
    Q_PROPERTY( int penalty READ penalty WRITE setPenaltyPoints )
    Q_PROPERTY( QTime startTime READ startTime WRITE setStartTime )
    Q_PROPERTY( QTime finishTime READ finishTime WRITE setFinishTime )
    Q_PROPERTY( QTime finalTime READ finalTime WRITE setFinalTime )

public:
    EventEntry( const QSqlRecord &record, const QString &table );
    QString name() const { return this->record().value( "name" ).toString(); }
    int minMembers() const { return this->record().value( "minMembers" ).toInt(); }
    int maxMembers() const { return this->record().value( "maxMembers" ).toInt(); }
    int api() const { return this->record().value( "api" ).toInt(); }
    int singleCombo() const { return this->record().value( "singleCombo" ).toInt(); }
    int doubleCombo() const { return this->record().value( "doubleCombo" ).toInt(); }
    int tripleCombo() const { return this->record().value( "tripleCombo" ).toInt(); }
    int penalty() const { return this->record().value( "penalty" ).toInt(); }
    QTime startTime() const { return QTime::fromString( this->record().value( "startTime" ).toString(), "hh:mm" ); }
    QTime finishTime() const { return QTime::fromString( this->record().value( "finishTime" ).toString(), "hh:mm" ); }
    QTime finalTime() const { return QTime::fromString( this->record().value( "finalTime" ).toString(), "hh:mm" ); }

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setMinMembers( int minMembers ) { this->setValue( "minMembers", minMembers ); }
    void setMaxMembers( int maxMembers ) { this->setValue( "maxMembers", maxMembers ); }
    void setAPIversion( int api ) { this->setValue( "api", api ); }
    void setSingleCombo( int points ) { this->setValue( "singleCombo", points ); }
    void setDoubleCombo( int points ) { this->setValue( "doubleCombo", points ); }
    void setTripleCombo( int points ) { this->setValue( "tripleCombo", points ); }
    void setPenaltyPoints( int points ) { this->setValue( "penalty", points ); }
    void setStartTime( QTime time ) { this->setValue( "startTime", time.toString( "hh:mm" )); }
    void setFinishTime( QTime time ) { this->setValue( "finishTime", time.toString( "hh:mm" )); }
    void setFinalTime( QTime time ) { this->setValue( "setFinalTime", time.toString( "hh:mm" )); }
};

#endif // EVENTENTRY_H
