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

    /**
     * @brief instance
     * @return
     */
    static Team *instance() { static Team *instance( new Team()); return instance; }
    virtual ~Team() override = default;

    Q_INVOKABLE Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    Row add( const QString &title, const QString &reviewer = QString());
    Q_INVOKABLE QString title( const Row &row ) const { return this->value( row, Title ).toString(); }

    /* TODO: removeme*/ Q_INVOKABLE QTime finishTime( const Row & ) const { return QTime(); }

    Q_INVOKABLE QString reviewer( const Row &row ) const { return this->value( row, Reviewer ).toString(); }
    Q_INVOKABLE Id eventId( const Row &row ) const { return static_cast<Id>( this->value( row, Event ).toInt()); }
    void removeOrphanedEntries() override;

public slots:
    void setTitle( const Row &row, const QString &title ) { this->setValue( row, Title, title ); }
    void setReviewer( const Row &row, const QString &name ) { this->setValue( row, Reviewer, name ); }

private:
    explicit Team();
};

// declare enums
Q_DECLARE_METATYPE( Team::Fields )
