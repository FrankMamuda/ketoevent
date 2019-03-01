/*
 * Copyright (C) 2018 Factory #12
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
#include <QTime>
#include "table.h"

/**
 * @brief The EventTable namespace
 */
namespace EventTable {
const static QString Name( "events" );
#ifdef Q_CC_MSVC
const static int Version = 10;
#else
const static __attribute__((unused)) int Version = 10;
#endif
}

/**
 * @brief The Event class
 */
class Event final : public Table {
    Q_OBJECT
    Q_ENUMS( Fields )
    Q_DISABLE_COPY( Event )
    friend class Task;
    friend class Team;

public:
    enum Fields {
        NoField = -1,
        ID,
        API,
        Title,
        Script,
        Options,

        // count
        Count
    };

    /**
     * @brief instance
     * @return
     */
    static Event *instance() { static Event *instance = new Event(); return instance; }
    virtual ~Event() override {}

    Row add( const QString &title, const QString &script = QString(), const QString &options = QString());
    Q_INVOKABLE Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    Q_INVOKABLE QString title( const Row &row ) const { return this->value( row, Title ).toString(); }
    QString script( const Row &row ) const;
    Q_INVOKABLE QString options( const Row &row ) const { return this->value( row, Options ).toString(); }

    void removeOrphanedEntries() override {}

public slots:
    void setTitle( const Row &row, const QString &title ) { this->setValue( row, Title, title ); }
    void setScript( const Row &row, const QString &script ) { this->setValue( row, Script, script ); }
    void setOptions( const Row &row, const QString &options ) { this->setValue( row, Options, options ); }

private:
    explicit Event();
};

// declare enums
Q_DECLARE_METATYPE( Event::Fields )

