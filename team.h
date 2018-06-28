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
 * @brief The TeamTable namespace
 */
namespace TeamTable {
const static QString Name( "teams" );
}

/**
 * @brief The Team class
 */
class Team final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Team )
    Q_ENUMS( Fields )
    friend class Log;

public:
    enum Fields {
        NoField = -1,
        ID,
        Title,
        Members,
        Finish,
        Lock,
        Reviewer,
        Event,

        // count
        Count
    };

    enum Types {
        NoType = -1,
        Check,
        Multi
    };

    enum Styles {
        NoStyle = -1,
        Regular,
        Bold,
        Italic
    };

    /**
     * @brief instance
     * @return
     */
    static Team *instance() { static Team *instance( new Team()); return instance; }
    virtual ~Team() = default;

    Id id( int row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    void add( const QString &title, int members, const QTime &finishTime, const QString &reviewer = QString());
    QString title( int row ) const { return this->value( row, Title ).toString(); }
    int members( int row ) const { return this->value( row, Members ).toInt(); }
    QTime finishTime( int row ) const { return QTime::fromString( this->value( row, Finish ).toString(), "hh:mm" ); }
    QString reviewer( int row ) const { return this->value( row, Reviewer ).toString(); }
    Id eventId( int row ) const { return static_cast<Id>( this->value( row, Event ).toInt()); }
    int eventRow( int row ) const;

public slots:
    void setTitle( int row, const QString &title ) { this->setValue( row, Title, title ); }
    void setMembers( int row, int members ) { this->setValue( row, Members, members ); }
    void setFinishTime( int row, const QTime &time ) { this->setValue( row, Finish, time.toString( "hh:mm" )); }
    void setReviewer( int row, const QString &name ) { this->setValue( row, Reviewer, name ); }

private:
    explicit Team();
};
