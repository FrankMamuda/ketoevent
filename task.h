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
 * @brief The TaskTable namespace
 */
namespace TaskTable {
const static QString Name( "tasks" );
}

/**
 * @brief The Task class
 */
class Task final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY( Task )
    Q_ENUMS( Fields )
    Q_ENUMS( Types )
    Q_ENUMS( Styles )
    friend class Log;
    friend class TaskEdit;

public:
    enum Fields {
        NoField = -1,
        ID,
        Name,
        Points,
        Mult,
        Style,
        Type,
        Order,
        Event,
        Desc,

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
    static Task *instance() { static Task *instance( new Task()); return instance; }
    virtual ~Task() {}

    Id id( int row ) const { return Id::fromInteger( this->value( row, ID ).toInt()); }
    void add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style = Task::NoStyle, const QString &description = QString());
    QString name( int row ) const { return this->value( row, Name ).toString(); }
    int points( int row ) const { return this->value( row, Points ).toInt(); }
    int multi( int row ) const { return this->value( row, Mult ).toInt(); }
    Styles style( int row ) const { return static_cast<Styles>( this->value( row, Style ).toInt()); }
    Types type( int row ) const { return static_cast<Types>( this->value( row, Type ).toInt()); }
    int order( int row ) const { return this->value( row, Order ).toInt(); }
    QString description( int row ) const { return this->value( row, Desc ).toString(); }
    Id eventId( int row ) const { return Id::fromInteger( this->value( row, Event ).toInt()); }
    int eventRow( int row ) const;
    QVariant data( const QModelIndex &idx, int role = Qt::DisplayRole ) const override;

public slots:
    void setName( int row, const QString &name ) { this->setValue( row, Name, name ); }
    void setPoints( int row, int points ) { this->setValue( row, Points, points ); }
    void setMulti( int row, int points ) { this->setValue( row, Mult, points ); }
    void setStyle( int row, Styles style ) { this->setValue( row, Style, static_cast<int>( style )); }
    void setType( int row, Types type ) { this->setValue( row, Type, static_cast<int>( type )); }
    void setOrder( int row, int position ) { this->setValue( row, Order, position ); }
    void setDescription( int row, const QString &description ) { this->setValue( row, Desc, description ); }

private:
    explicit Task();
    QMap<Types,QString> types;
    QMap<Styles,QString> styles;
};
