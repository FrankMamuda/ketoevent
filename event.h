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
#include <QTime>
#include "table.h"

/**
 * @brief The EventTable namespace
 */
namespace EventTable {
[[maybe_unused]] constexpr static int Version = 9;
[[maybe_unused]] constexpr static int DefaultMinMembers = 1;
[[maybe_unused]] constexpr static int DefaultMembers = 2;
[[maybe_unused]] constexpr static int DefaultMaxMembers = 3;
[[maybe_unused]] constexpr static int DefaultComboOfTwo = 1;
[[maybe_unused]] constexpr static int DefaultComboOfThree = 3;
[[maybe_unused]] constexpr static int DefaultComboOfFourAndMore = 5;
[[maybe_unused]] constexpr static int DefaultPenaltyPoints = 5;
[[maybe_unused]] constexpr static const char *DefaultStartTime = "10:00";
[[maybe_unused]] constexpr static const char *DefaultFinishTime = "15:00";
[[maybe_unused]] constexpr static const char *DefaultFinalTime = "15:30";
}

/**
 * @brief The Event class
 */
class Event final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( Event )
    friend class Task;
    friend class Team;

public:
    enum Fields {
        NoField = -1,
        ID,
        API,
        Title,
        Min,
        Max,
        Start,
        Finish,
        Final,
        Penalty,
        Combo2,
        Combo3,
        Combo4,
        Lock,

        // count
        Count
    };
    Q_ENUM( Fields )

    /**
     * @brief instance
     * @return
     */
    static Event& instance() { static Event instance; return instance; }
    ~Event() override {}

    Row add( const QString &title, int minMembers = EventTable::DefaultMinMembers, int maxMembers = EventTable::DefaultMaxMembers,
            const QTime &start = QTime::fromString( EventTable::DefaultStartTime, Database_::TimeFormat ),
            const QTime &finish = QTime::fromString( EventTable::DefaultFinishTime, Database_::TimeFormat ),
            const QTime &final  = QTime::fromString( EventTable::DefaultFinalTime, Database_::TimeFormat ),
            int penalty = EventTable::DefaultPenaltyPoints,
            int two = EventTable::DefaultComboOfTwo, int three = EventTable::DefaultComboOfThree, int fourPlus = EventTable::DefaultComboOfFourAndMore );
    [[nodiscard]] Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    [[nodiscard]] QString title( const Row &row ) const { return this->value( row, Title ).toString(); }
    [[nodiscard]] int minMembers( const Row &row ) const { return this->value( row, Min ).toInt(); }
    [[nodiscard]] int maxMembers( const Row &row ) const { return this->value( row, Max ).toInt(); }
    [[nodiscard]] QTime startTime( const Row &row ) const { return QTime::fromString( this->value( row, Start ).toString(), Database_::TimeFormat ); }
    [[nodiscard]] QTime finishTime( const Row &row ) const { return QTime::fromString( this->value( row, Finish ).toString(), Database_::TimeFormat ); }
    [[nodiscard]] QTime finalTime( const Row &row ) const { return QTime::fromString( this->value( row, Final ).toString(), Database_::TimeFormat ); }
    [[nodiscard]] int penalty( const Row &row ) const { return this->value( row, Penalty ).toInt(); }
    [[nodiscard]] int comboOfTwo( const Row &row ) const { return this->value( row, Combo2 ).toInt(); }
    [[nodiscard]] int comboOfThree( const Row &row ) const { return this->value( row, Combo3 ).toInt(); }
    [[nodiscard]] int comboOfFourPlus( const Row &row ) const { return this->value( row, Combo4 ).toInt(); }

    void removeOrphanedEntries() override {}

public slots:
    void setTitle( const Row &row, const QString &title ) { this->setValue( row, Title, title ); }
    void setMinMembers( const Row &row, int minMembers ) { this->setValue( row, Min, minMembers ); }
    void setMaxMembers( const Row &row, int maxMembers ) { this->setValue( row, Max, maxMembers ); }
    void setStartTime( const Row &row, const QTime &time ) { this->setValue( row, Start, time.toString( Database_::TimeFormat )); }
    void setFinishTime( const Row &row, const QTime &time ) { this->setValue( row, Finish, time.toString( Database_::TimeFormat )); }
    void setFinalTime( const Row &row, const QTime &time ) { this->setValue( row, Final, time.toString( Database_::TimeFormat )); }
    void setPenaltyPoints( const Row &row, int points ) { this->setValue( row, Penalty, points ); }
    void setComboOfTwo( const Row &row, int points ) { this->setValue( row, Combo2, points ); }
    void setComboOfThree( const Row &row, int points ) { this->setValue( row, Combo3, points ); }
    void setComboOfFourPlus( const Row &row, int points ) { this->setValue( row, Combo4, points ); }

private:
    explicit Event();
};

// declare enums
Q_DECLARE_METATYPE( Event::Fields )

