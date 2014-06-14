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

#ifndef DATABASEENTRY_H
#define DATABASEENTRY_H

//
// includes
//
#include <QObject>
#include <QSqlRecord>
#include <QVariant>

//
// inherited by EventEntry, LogEntry, TaskEntry, TeamEntry
//

//
// class: DatabaseEntry
//
class DatabaseEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY( int id READ id )
    Q_PROPERTY( QString table READ table WRITE setTable )
    Q_PROPERTY( QSqlRecord record READ record WRITE setRecord )
    Q_PROPERTY( bool imported READ isImported WRITE setImported )
    Q_CLASSINFO( "description", "SQL Entry" )

public:
    DatabaseEntry() { this->setImported( false ); }
    ~DatabaseEntry() { this->m_table.clear(); this->m_record.clear(); }
    int id () const { return this->record().value( "id" ).toInt(); }
    QSqlRecord record() const { return this->m_record; }
    QString table() const { return this->m_table; }
    bool isImported() const { return this->m_import; }

public slots:
    void setTable( const QString &name ) { this->m_table = name; }
    void setRecord( const QSqlRecord &record ) { this->m_record = record; }
    void setValue( const QString &name, const QVariant &value );
    void store();
    void setImported( bool import = true ) { this->m_import = import; }

private:
    QString m_table;
    QSqlRecord m_record;
    bool m_import;

signals:
    void changed();
};

#endif // DATABASEENTRY_H
