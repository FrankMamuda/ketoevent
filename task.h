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
#include <QIdentityProxyModel>

/**
 * @brief The Task class
 */
class Task final : public Table {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( Task )
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
    Q_ENUM( Fields )

    enum ExtendedFields {
        ComboID = Count,
        Multi,

        // count
        ExtendedCount
    };
    Q_ENUM( ExtendedFields )

    enum class Types {
        NoType = -1,
        Check,
        Multi
    };
    Q_ENUM( Types )

    enum class Styles {
        NoStyle = -1,
        Regular,
        Bold,
        Italic
    };
    Q_ENUM( Styles )

    /**
     * @brief instance
     * @return
     */
    static Task *instance() { if ( Task::i == nullptr ) Task::i = new Task(); return Task::i; }
    ~Task() override { this->setInitialised( false ); }

    [[nodiscard]] Id id( const Row &row ) const { return static_cast<Id>( this->value( row, ID ).toInt()); }
    Row add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style = Styles::NoStyle, const QString &description = QString());
    [[nodiscard]] QString name( const Row &row ) const { return this->value( row, Name ).toString(); }
    [[nodiscard]] int points( const Row &row ) const { return this->value( row, Points ).toInt(); }
    [[nodiscard]] int multi( const Row &row ) const { return this->value( row, Mult ).toInt(); }
    [[nodiscard]] Styles style( const Row &row ) const { return static_cast<Styles>( this->value( row, Style ).toInt()); }
    [[nodiscard]] Types type( const Row &row ) const { return static_cast<Types>( this->value( row, Type ).toInt()); }
    [[nodiscard]] int order( const Row &row ) const { return this->value( row, Order ).toInt(); }
    [[nodiscard]] QString description( const Row &row ) const { return this->value( row, Desc ).toString(); }
    [[nodiscard]] Id eventId( const Row &row ) const { return static_cast<Id>( this->value( row, Event ).toInt()); }
    [[nodiscard]] QVariant data( const QModelIndex &idx, int role = Qt::DisplayRole ) const override;
    [[nodiscard]] int multiplier( const Row &row ) const;
    [[nodiscard]] Id comboId( const Row &row ) const;
    [[nodiscard]] QPair<Id, Id>getIds( const Row &row, bool *ok ) const;

    [[nodiscard]] bool hasInitialised() const { return this->m_initialised; }

    void removeOrphanedEntries() override;

    [[nodiscard]] int columnCount( const QModelIndex & = QModelIndex()) const override { return ExtendedCount; }

public slots:
    void setName( const Row &row, const QString &name ) { this->setValue( row, Name, name ); }
    void setPoints( const Row &row, int points ) { this->setValue( row, Points, points ); }
    void setMulti( const Row &row, int points ) { this->setValue( row, Mult, points ); }
    void setStyle( const Row &row, Task::Styles style ) { this->setValue( row, Style, static_cast<int>( style )); }
    void setType( const Row &row, Task::Types type ) { this->setValue( row, Type, static_cast<int>( type )); }
    void setOrder( const Row &row, int position ) { this->setValue( row, Order, position ); }
    void setDescription( const Row &row, const QString &description ) { this->setValue( row, Desc, description ); }
    void setMultiplier( const Row &row, int value );
    void setInitialised( bool initialised = true ) { this->m_initialised = initialised; }

protected:
    QString selectStatement() const override;

private:
    static Task *i;
    explicit Task();
    QMap<Types,QString> types;
    QMap<Styles,QString> styles;
    bool m_initialised = false;
};

// declare enums
Q_DECLARE_METATYPE( Task::Fields )
Q_DECLARE_METATYPE( Task::Types )
Q_DECLARE_METATYPE( Task::Styles )

/**
 * @brief The TaskProxyModel class
 */
class TaskProxyModel : public QIdentityProxyModel {
public:
    /**
     * @brief instance
     * @return
     */
    static TaskProxyModel *instance() { if ( TaskProxyModel::i == nullptr ) TaskProxyModel::i = new TaskProxyModel(); return TaskProxyModel::i; }
    ~TaskProxyModel() override {}
    QVariant data( const QModelIndex &index, int role ) const override;

private:
    static TaskProxyModel *i;
    explicit TaskProxyModel();
};
