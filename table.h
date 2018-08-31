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
 * @brief The Id enum strong-typed id
 */
enum class Id : int { Invalid = -1 };

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
    bool hasPrimaryField() const { return this->m_hasPrimary; }
    int count() const;
    QVariant value( int row, int fieldId ) const;
    bool contains( int fieldId, const QVariant &value ) const { return this->contains( this->field( fieldId ), value ); }
    virtual bool select() override;
    QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    virtual QVariant data( const QModelIndex &item, int role ) const override;
    int row( const Id &id ) const { if ( this->map.contains( id )) return this->map[id].row(); return -1; }
    virtual void setFilter( const QString &filter ) override;
    QString fieldName( int id ) const;

public slots:
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid, const QString &format = QString( "text" ), bool unique = false, bool autoValue = false );
    Id add( const QVariantList &arguments );
    void remove( int row );
    void setValue( int row, int fieldId, const QVariant &value );

protected:
    QMap<int, QSharedPointer<Field_>> fields;
    QMap<Id, QPersistentModelIndex> map;
    QSharedPointer<Field_> field( int id ) const;
    bool contains( const QSharedPointer<Field_> &field, const QVariant &value ) const;

private:
    bool m_valid;
    bool m_hasPrimary;
    QSharedPointer<Field_> m_primaryField;
};