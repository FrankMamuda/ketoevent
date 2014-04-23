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

#ifndef LOGENTRY_H
#define LOGENTRY_H

//
// includes
//
#include "databaseentry.h"

//
// class: LogEntry
//
class LogEntry : public DatabaseEntry {
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( int check READ check )
    Q_PROPERTY( int points READ points )
    Q_PROPERTY( int teamId READ teamId WRITE setTeamId )
    Q_PROPERTY( int taskId READ taskId WRITE setTaskId )
    Q_PROPERTY( int comboId READ comboId WRITE setComboId )

public:
    LogEntry( const QSqlRecord &record, const QString &table );
    int value() const { return this->record().value( "value" ).toInt(); }
    bool check() const { return static_cast<bool>( this->value()); }
    int points() const;
    int teamId() const { return this->record().value( "teamId" ).toInt(); }
    int taskId() const { return this->record().value( "taskId" ).toInt(); }
    int comboId() const { return this->record().value( "comboId" ).toInt(); }

signals:
    void comboIdChanged();

public slots:
    void setValue( int value ) { this->DatabaseEntry::setValue( "value", value ); }
    void setTeamId( int id ) { this->DatabaseEntry::setValue( "teamId", id ); }
    void setTaskId( int id ) { this->DatabaseEntry::setValue( "taskId", id ); }
    void setComboId( int id ) { this->DatabaseEntry::setValue( "comboId", id ); emit this->comboIdChanged(); }
};

#endif // LOGENTRY_H
