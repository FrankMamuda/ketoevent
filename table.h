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
class QDebug;

/**
 * @brief The Id enum strong-typed id
 */
enum class Id : int { Invalid = -1 };
QDebug operator<<( QDebug debug, const Id &id );

/**
 * @brief The Row enum strong-typed row
 */
enum class Row : int { Invalid = -1 };
QDebug operator<<( QDebug debug, const Row &row );

/**
 * @brief The Table_ class
 */
class Table : public QSqlTableModel {
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
    QVariant value( Row row, int fieldId ) const;
    bool contains( int fieldId, const QVariant &value ) const { return this->contains( this->field( fieldId ), value ); }
    virtual bool select() override;
    QSharedPointer<Field_> primaryField() const { return this->m_primaryField; }
    virtual QVariant data( const QModelIndex &item, int role ) const override;
    Row row( const Id &id ) const { if ( this->map.contains( id )) return this->indexToRow( this->map[id].row()); return Row::Invalid; }
    virtual void setFilter( const QString &filter ) override;
    QString fieldName( int id ) const;
    Row indexToRow( const int index ) const { if ( index < 0 || index >= this->count()) return Row::Invalid; return static_cast<Row>( index ); }
    Row indexToRow( const QModelIndex &index ) const { if ( index.row() < 0 || index.row() >= this->count() || index.model() != this ) return Row::Invalid; return static_cast<Row>( index.row()); }

public slots:
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QVariant::Type type = QVariant::Invalid, const QString &format = QString( "text" ), bool unique = false, bool autoValue = false );
    Id add( const QVariantList &arguments );
    void remove( Row row );
    void setValue( Row row, int fieldId, const QVariant &value );

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

// declare enums
Q_DECLARE_METATYPE( Table::Roles )
Q_DECLARE_METATYPE( Id )
Q_DECLARE_METATYPE( Row )
