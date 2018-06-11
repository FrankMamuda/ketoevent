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
    bool select();
    QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    QVariant data( const QModelIndex &item, int role ) const;
    int row( int id ) const { if ( this->map.contains( id )) return this->map[id].row(); return -1; }

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
