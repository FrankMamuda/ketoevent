/*
 * Copyright (C) 2013-2018 Factory #12
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

#pragma once

//
// includes
//
#include <QObject>
#include <QSqlRecord>
#include <QVariant>

/**
 * @brief The DatabaseEntry class
 */
class DatabaseEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY( int id READ id )
    Q_PROPERTY( QString table READ table WRITE setTable )
    Q_PROPERTY( QSqlRecord record READ record WRITE setRecord )
    Q_CLASSINFO( "description", "SQL Entry" )

public:
    DatabaseEntry() {}
    ~DatabaseEntry() { this->m_table.clear(); this->m_record.clear(); }
    int id () const { return this->record().value( "id" ).toInt(); }
    QSqlRecord record() const { return this->m_record; }
    QString table() const { return this->m_table; }

public slots:
    void setTable( const QString &name ) { this->m_table = name; }
    void setRecord( const QSqlRecord &record ) { this->m_record = record; }
    void setValue( const QString &name, const QVariant &value );

private:
    QString m_table;
    QSqlRecord m_record;

signals:
    void changed();
};
