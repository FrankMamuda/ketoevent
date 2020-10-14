/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "table.h"

/**
 * @brief The Log class
 */
class Log final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Log )

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
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Log *instance() { static Log *instance( new Log()); return instance; }
    ~Log() override {}

    // disable move
    Log( Log&& ) = delete;
    Log& operator=( Log&& ) = delete;

    void add( const Id &taskId, const Id &teamId, int multiplier = 0, const Id &comboId = Id::Invalid );
    [[nodiscard]] Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    [[nodiscard]] Id id( const Id &taskId, const Id &teamId ) const;
    [[nodiscard]] int multiplier( const Row &row ) const { return this->value( row, Multi ).toInt(); }
    [[nodiscard]] int multiplier( const Id &taskId, const Id &teamId ) const;
    [[nodiscard]] Id taskId( const Row &row ) const { return static_cast<Id>( this->value( row, Task ).toInt()); }
    [[nodiscard]] Id teamId( const Row &row ) const { return static_cast<Id>( this->value( row, Team ).toInt()); }
    [[nodiscard]] Id comboId( const Row &row ) const { return static_cast<Id>( this->value( row, Combo ).toInt()); }
    [[nodiscard]] Id comboId( const Id &taskId, const Id &teamId ) const;

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
