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

#ifndef TASKENTRY_H
#define TASKENTRY_H

//
// includes
//
#include "databaseentry.h"

//
// class: TaskEntry
//
class TaskEntry : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int points READ points WRITE setPoints )
    Q_PROPERTY( int multi READ multi WRITE setMulti )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( bool challenge READ isChallenge WRITE setChallenge )
    Q_PROPERTY( int order READ order WRITE setOrder )
    Q_ENUMS( Types )

public:
    enum Types {
        NoType = -1,
        Check,
        Multi,
        Special
    };
    explicit TaskEntry( const QSqlRecord &record, const QString &table );
    QString name() const { return this->record().value( "name" ).toString(); }
    int points() const { return this->record().value( "points" ).toInt(); }
    int multi() const { return this->record().value( "multi" ).toInt(); }
    Types type() const { return static_cast<Types>( this->record().value( "type" ).toInt()); }
    bool isChallenge() const { if ( this->type() == Special ) return false; else return this->record().value( "challenge" ).toBool(); }
    int order() const { return this->record().value( "parent" ).toInt(); }

public slots:
    int calculate( int logId ) const;
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setPoints( int points = 0 ) { this->setValue( "points", points ); }
    void setMulti( int multi = 0 ) { this->setValue( "multi", multi ); }
    void setType( Types type = Check ) { this->setValue( "type", static_cast<int>( type )); }
    void setChallenge( bool challenge ) { this->setValue( "challenge", static_cast<int>( challenge )); }
    void setOrder( int order = 0 ) { if ( order >= 0 ) this->setValue( "parent", order ); }
};

#endif // TASKENTRY_H
