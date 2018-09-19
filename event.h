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
const static int Version = 9;
const static int DefaultMinMembers = 1;
const static int DefaultMembers = 2;
const static int DefaultMaxMembers = 3;
const static int DefaultComboOfTwo = 1;
const static int DefaultComboOfThree = 3;
const static int DefaultComboOfFourAndMore = 5;
const static int DefaultPenaltyPoints = 5;
#else
const static __attribute__((unused)) int Version = 9;
const static __attribute__((unused)) int DefaultMinMembers = 1;
const static __attribute__((unused)) int DefaultMembers = 2;
const static __attribute__((unused)) int DefaultMaxMembers = 3;
const static __attribute__((unused)) int DefaultComboOfTwo = 1;
const static __attribute__((unused)) int DefaultComboOfThree = 3;
const static __attribute__((unused)) int DefaultComboOfFourAndMore = 5;
const static __attribute__((unused)) int DefaultPenaltyPoints = 5;
#endif
const static QString DefaultStartTime( "10:00" );
const static QString DefaultFinishTime( "15:00" );
const static QString DefaultFinalTime( "15:30" );
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

    /**
     * @brief instance
     * @return
     */
    static Event *instance() { static Event *instance = new Event(); return instance; }
    virtual ~Event() {}

    Row add( const QString &title, int minMembers = EventTable::DefaultMinMembers, int maxMembers = EventTable::DefaultMaxMembers,
            const QTime &start = QTime::fromString( EventTable::DefaultStartTime, Database_::TimeFormat ),
            const QTime &finish = QTime::fromString( EventTable::DefaultFinishTime, Database_::TimeFormat ),
            const QTime &final  = QTime::fromString( EventTable::DefaultFinalTime, Database_::TimeFormat ),
            int penalty = EventTable::DefaultPenaltyPoints,
            int two = EventTable::DefaultComboOfTwo, int three = EventTable::DefaultComboOfThree, int fourPlus = EventTable::DefaultComboOfFourAndMore );
    Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    QString title( const Row &row ) const { return this->value( row, Title ).toString(); }
    int minMembers( const Row &row ) const { return this->value( row, Min ).toInt(); }
    int maxMembers( const Row &row ) const { return this->value( row, Max ).toInt(); }
    QTime startTime( const Row &row ) const { return QTime::fromString( this->value( row, Start ).toString(), Database_::TimeFormat ); }
    QTime finishTime( const Row &row ) const { return QTime::fromString( this->value( row, Finish ).toString(), Database_::TimeFormat ); }
    QTime finalTime( const Row &row ) const { return QTime::fromString( this->value( row, Final ).toString(), Database_::TimeFormat ); }
    int penalty( const Row &row ) const { return this->value( row, Penalty ).toInt(); }
    int comboOfTwo( const Row &row ) const { return this->value( row, Combo2 ).toInt(); }
    int comboOfThree( const Row &row ) const { return this->value( row, Combo3 ).toInt(); }
    int comboOfFourPlus( const Row &row ) const { return this->value( row, Combo4 ).toInt(); }

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

