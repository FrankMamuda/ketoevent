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
    Q_ENUMS( DataRoles )
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
        Tag,

        // count
        Count
    };

    enum ExtendedFields {
        ComboID = Count,
        Multi,

        // count
        ExtendedCount
    };

    enum class Types {
        NoType = -1,
        Check,
        Multi
    };

    enum class Styles {
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
    virtual ~Task() override { this->setInitialised( false ); }

    Q_INVOKABLE Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    Row add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style = Styles::NoStyle, const QString &description = QString(), const QString &tag = QString());
    Q_INVOKABLE QString name( const Row &row ) const { return this->value( row, Name ).toString(); }
    Q_INVOKABLE int points( const Row &row ) const { return this->value( row, Points ).toInt(); }
    Q_INVOKABLE int multi( const Row &row ) const { return this->value( row, Mult ).toInt(); }
    Q_INVOKABLE Styles style( const Row &row ) const { return static_cast<Styles>( this->value( row, Style ).toInt()); }
    Q_INVOKABLE Types type( const Row &row ) const { return static_cast<Types>( this->value( row, Type ).toInt()); }
    Q_INVOKABLE int order( const Row &row ) const { return this->value( row, Order ).toInt(); }
    Q_INVOKABLE QString description( const Row &row ) const { return this->value( row, Desc ).toString(); }
    Q_INVOKABLE Id eventId( const Row &row ) const { return static_cast<Id>( this->value( row, Event ).toInt()); }
    QVariant data( const QModelIndex &idx, int role = Qt::DisplayRole ) const override;
    Q_INVOKABLE int multiplier( const Row &row ) const;
    Q_INVOKABLE Id comboId( const Row &row ) const;
    QPair<Id, Id>getIds( const Row &row, bool *ok ) const;

    bool hasInitialised() const { return this->m_initialised; }

    void removeOrphanedEntries() override;

    int columnCount( const QModelIndex & = QModelIndex()) const override { return ExtendedCount; }

public slots:
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }
    void setPoints( const Row &row, int points ) { this->setValue( row, Points, points ); }
    void setMulti( const Row &row, int points ) { this->setValue( row, Mult, points ); }
    void setStyle( const Row &row, Styles style ) { this->setValue( row, Style, static_cast<int>( style )); }
    void setType( const Row &row, Types type ) { this->setValue( row, Type, static_cast<int>( type )); }
    void setOrder( const Row &row, int position ) { this->setValue( row, Order, position ); }
    void setDescription( const Row &row, const QString &description ) { this->setValue( row, Desc, description ); }
    void setMultiplier( const Row &row, int value );

    void setInitialised( bool initialised = true ) { this->m_initialised = initialised; }

protected:
    QString selectStatement() const override;

private:
    explicit Task();
    QMap<Types,QString> types;
    QMap<Styles,QString> styles;
    bool m_initialised = false;
};

// declare enums
Q_DECLARE_METATYPE( Task::Fields )
Q_DECLARE_METATYPE( Task::Types )
Q_DECLARE_METATYPE( Task::Styles )
