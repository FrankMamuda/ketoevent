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
#include "table.h"

/**
 * @brief The LogTable namespace
 */
namespace LogTable {
const static QString Name( "logs" );
}

/**
 * @brief The Log class
 */
class Log final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Log )
    Q_ENUMS( Fields )
    Q_ENUMS( Roles )

public:
    enum Fields {
        NoField = -1,
        ID,
        Multi,
        Task,
        Team,
        Combo,

        // count
        Count
    };

    /**
     * @brief instance
     * @return
     */
    static Log *instance() { static Log *instance( new Log()); return instance; }
    virtual ~Log() override {}

    void add( const Id &taskId, const Id &teamId, int multiplier = 0, const Id &comboId = Id::Invalid );
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    Id id( const Id &taskId, const Id &teamId ) const;
    int multiplier( const Row &row ) const { return this->value( row, Multi ).toInt(); }
    int multiplier( const Id &taskId, const Id &teamId ) const;
    Id taskId( const Row &row ) const { return static_cast<Id>( this->value( row, Task ).toInt()); }
    Id teamId( const Row &row ) const { return static_cast<Id>( this->value( row, Team ).toInt()); }
    Id comboId( const Row &row ) const { return static_cast<Id>( this->value( row, Combo ).toInt()); }
    Id comboId( const Id &taskId, const Id &teamId ) const;

    void removeOrphanedEntries() override;

public slots:
    void setMultiplier( const Row &row, int multi ) { this->setValue( row, Multi, multi ); }
    void setMultiplier( int multi, const Id &taskId, const Id &teamId );
    void setTaskId( const Row &row, const Id &id ) { this->setValue( row, Task, static_cast<int>( id )); }
    void setTeamId( const Row &row, const Id &id ) { this->setValue( row, Team, static_cast<int>( id )); }
    void setComboId( const Row &row, const Id &id ) { this->setValue( row, Combo, static_cast<int>( id )); }

private:
    explicit Log();
};

// declare enums
Q_DECLARE_METATYPE( Log::Fields )
