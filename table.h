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
#include <QSqlRelationalTableModel>
#include <QSharedPointer>
#include <QSqlRecord>

//
// classes
//
class Field_;

/**
 * @brief The Id class used to avoid row/id mixups
 */
class Id {
public:
    explicit Id() = default;
    Id( const Id &s ) : m_value( s.value()) {}
    Id &operator=( const Id &s ) { this->m_value = s.value(); return *this; }
    int value() const { return this->m_value; }
    void setValue( const int v ) { this->m_value = v; }
    static Id fromInteger( const int v ) { Id id; id.setValue( v ); return id; }
    bool operator==( const Id &other ) const { return this->value() == other.value(); }
    bool operator>( const Id &other ) const { return this->value() > other.value(); }
    bool operator<( const Id &other ) const { return this->value() < other.value(); }
    bool operator>=( const Id &other ) const { return this->value() >= other.value(); }
    bool operator<=( const Id &other ) const { return this->value() <= other.value(); }

private:
    int m_value;
};

/**
 * @brief The Table_ class
 */
class Table : public QSqlRelationalTableModel {
    Q_OBJECT
    Q_ENUMS( Roles )
    Q_DISABLE_COPY( Table )
    friend class Database;

public:
    enum Roles {
        IDRole = Qt::UserRole
    };

    Table( const QString &tableName = QString());
    virtual ~Table() { this->setValid( false ); this->clear(); }
    bool isValid() const { return this->m_valid; }
    int count() const { return this->rowCount(); }
    QVariant value( int row, int fieldId ) const;
    bool contains( int fieldId, const QVariant &value ) const { return this->contains( this->field( fieldId ), value ); }
    virtual bool select() override;
    QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    virtual QVariant data( const QModelIndex &item, int role ) const override;
    int row( const Id &id ) const { if ( this->map.contains( id.value())) return this->map[id.value()].row(); return -1; }
    virtual void setFilter( const QString &filter ) override;

public slots:
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid, const QString &format = QString( "text" ), bool unique = false, bool autoValue = false );
    void add( const QVariantList &arguments );
    void remove( int row );
    void setValue( int row, int fieldId, const QVariant &value );

protected:
    QMap<int, QSharedPointer<Field_>> fields;
    QMap<int, QPersistentModelIndex> map;
    QSharedPointer<Field_> field( int id ) const;
    bool contains( const QSharedPointer<Field_> &field, const QVariant &value ) const;

private:
    bool m_valid;
    QSharedPointer<Field_> m_primaryField;
};
