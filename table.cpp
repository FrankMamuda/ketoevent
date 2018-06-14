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

//
// includes
//
#include <QSqlRecord>
#include "table.h"
#include "database.h"
#include "field.h"
#include <QDebug>

/**
 * @brief Table_::Table_
 * @param name
 * @param map
 */
Table::Table( const QString &name ) : m_valid( false ) {
    this->setTable( name );
}

/**
 * @brief Table::value
 * @param row
 * @param fieldId
 * @return
 */
QVariant Table::value( int row, int fieldId ) const {
    if ( !this->isValid())
        return -1;

    const QModelIndex index( this->index( row, fieldId ));
    if ( !index.isValid() || index.row() < 0 || index.row() >= this->count()) {
        qWarning( Database_::Debug ) << this->tr( "could not retrieve field \"%1\" value from table \"%2\"" )
                                        .arg( this->field( fieldId )->name())
                                        .arg( this->tableName()) << !index.isValid() << ( index.row() < 0 ) << ( index.row() >= this->count());
        return -1;
    }

    return QSqlRelationalTableModel::data( index );
}

/**
 * @brief Table::select
 * @return
 */
bool Table::select() {
    int y;
    const bool result = QSqlRelationalTableModel::select();

    // fetch more
    while ( this->canFetchMore())
        this->fetchMore();

    // find primary key
    foreach ( const Field &field, this->fields ) {
        if ( field->isPrimary())
            this->m_primaryField = field;
    }

    // build id to row map
    this->map.clear();
    if ( !this->primaryField().isNull()) {
        for ( y = 0; y < this->count(); y++ )
            this->map[this->record( y ).value( this->primaryField()->id()).toInt()] = QPersistentModelIndex( this->index( y, this->primaryField()->id()));
    }

    return result;
}

/**
 * @brief Table::data
 * @param item
 * @param role
 * @return
 */
QVariant Table::data( const QModelIndex &item, int role ) const {
    if ( role == IDRole ) {
        if ( !this->primaryField()->isNull())
            return this->record( item.row()).value( this->primaryField()->id()).toInt();
    }

    return QSqlRelationalTableModel::data( item, role );
}

/**
 * @brief Table::setFilter
 * @param filter
 */
void Table::setFilter( const QString &filter ) {
    QSqlTableModel::setFilter( filter );
    this->select();
}

/**
 * @brief Table_::fieldFromId
 * @param id
 * @return
 */
Field Table::field( int id ) const {
    if ( this->fields.contains( id ))
        return this->fields[id];

    return Field();
}

/**
 * @brief Table_::addField
 * @param id
 * @param fieldName
 * @param type
 * @param format
 * @param unique
 * @param autoValue
 */
void Table::addField( int id, const QString &fieldName, QVariant::Type type, const QString &format, bool unique, bool autoValue ) {
    if ( this->fields.contains( id ))
        return;

    this->fields[id] = Field( new Field_( id, fieldName, type, format, unique, autoValue ));
}

/**
 * @brief Table::add
 * @param name
 */
void Table::add( const QVariantList &arguments ) {
    int y, row;

    if ( !this->isValid())
        return;

    if ( this->fields.count() != arguments.count())
        qCCritical( Database_::Debug ) << this->tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg( this->fields.count());

    // insert empty row
    row = this->count();
    this->insertRow( row );

    // prepare statement
    for ( y = 0; y < arguments.count(); y++ ) {
        Field field;
        QVariant argument;

        // get field and argument
        field = this->fields[y];
        argument = arguments.at( y );

        // compare types
        if ( field->type() != argument.type()) {
            qCCritical( Database_::Debug ) << this->tr( "incompatible field type - %1 for argument %2 (%3), required - %4" ).arg( argument.type()).arg( y ).arg( field->format()).arg( field->type());
            this->revert();
            return;
        }

        // check for unique fields
        if ( field->isUnique() && !field->isAutoValue()) {
            if ( this->contains( field, argument )) {
                qCWarning( Database_::Debug )
                        << this->tr( "table already has a unique field \"%1\" with value - \"%2\", aborting addition" )
                           .arg( field->name()).arg( argument.toString());
                this->revert();
                return;
            }
        }

        // add to id/row map
        if ( field->isPrimary())
            this->map[argument.toInt()] = QPersistentModelIndex( this->index( row, field->id()));
        else
            this->setData( this->index( this->count() - 1, y ), argument );
    }

    this->submit();
}

/**
 * @brief Table::remove
 * @param row
 */
void Table::remove( int row ) {
    if ( !this->isValid())
        return;

    if ( !this->primaryField().isNull())
        this->map.remove( this->record( row ).value( this->primaryField()->id()).toInt());

    this->removeRow( row );
    this->select();
}

/**
 * @brief Table::setValue
 * @param row
 * @param fieldId
 */
void Table::setValue( int row, int fieldId, const QVariant &value ) {
    if ( !this->isValid())
        return;

    this->setData( this->index( row, fieldId ), value );
    this->submit();
}

/**
 * @brief Table::contains
 * @param field
 * @param value
 * @return
 */
bool Table::contains( const QSharedPointer<Field_> &field, const QVariant &value ) const {
    int y;

    if ( !this->isValid())
        return false;

    for ( y = 0; y < this->count(); y++ ) {
        const QSqlRecord record = this->record( y );
        if ( record.value( field->id()) == value )
            return true;
    }
    return false;
}
