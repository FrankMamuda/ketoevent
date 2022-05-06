/*
 * Copyright (C) 2018-2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "table.h"
#include "database.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

/*
 * FieldTypes
 */
QMultiMap<QMetaType::Type, QString> Table::FieldTypes = {
    { QMetaType::Bool,   "INTEGER" },
    { QMetaType::Int,    "INTEGER" },
    { QMetaType::UInt,   "INTEGER" },
    { QMetaType::Long,   "INTEGER" },
    { QMetaType::Short,  "INTEGER" },
    { QMetaType::ULong,  "INTEGER" },
    { QMetaType::UShort, "INTEGER" },

    { QMetaType::Float,  "REAL" },
    { QMetaType::Double, "REAL" },

    { QMetaType::QString, "TEXT" },

    { QMetaType::QByteArray, "BLOB" }
};

/**
 * @brief Table::count
 * @return
 */
int Table::count() const {
    return Database::instance()->hasInitialised() ? this->rowCount() : 0;
}

/**
 * @brief Table::value
 * @param row
 * @param fieldId
 * @return
 */
QVariant Table::value( const Row &row, int fieldId ) const {
    if ( !this->isValid())
        return -1;

    const QModelIndex index( this->index( static_cast<int>( row ), fieldId ));
    if ( row == Row::Invalid || !index.isValid() || index.row() < 0 || index.row() >= this->count()) {
        qWarning( Database_::Debug )
                << Table::tr( R"(could not retrieve field "%1" value from table "%2")" ).arg( this->field( fieldId ).name(), this->tableName())
                << !index.isValid()
                << ( index.row() < 0 )
                << ( index.row() >= this->count()) << row;
        return -1;
    }

    return QSqlTableModel::data( index );
}


/**
 * @brief Table::value this only works if table has a primary key
 * @param id
 * @param fieldId
 * @return
 */
QVariant Table::value( const Id &id, int fieldId ) const {
    if ( !this->isValid())
        return -1;

    if ( !this->hasPrimaryField())
        return -1;

    QSqlQuery query;
    query.exec( QString( "select %1, %2 from %3 where %1=%4" )
                        .arg( this->fieldName( this->primaryFieldIndex ),
                              this->fieldName( fieldId ),
                              this->tableName(),
                              QString::number( static_cast<int>( id ))));

    return query.next() ? query.value( 1 ) : "";
}

/**
 * @brief Table::select
 * @return
 */
bool Table::select() {
    const bool result = QSqlTableModel::select();

    // fetch more
    while ( this->canFetchMore())
        this->fetchMore();

    return result;
}

/**
 * @brief Table::row
 * @param id
 * @return
 */
Row Table::row( const Id &id ) const {
    const QModelIndexList list( this->match( this->index( 0, 0 ), IDRole, static_cast<int>( id ), 1, Qt::MatchExactly ));
    return this->row( list.isEmpty() ? QModelIndex() : list.first());
}

/**
 * @brief Table::data
 * @param index
 * @param role
 * @return
 */
QVariant Table::data( const QModelIndex &index, int role ) const {
    if ( !Database::instance()->hasInitialised())
        return QVariant();

    if ( role == IDRole || role == Qt::UserRole ) {
        if ( !index.isValid())
            return static_cast<int>( Id::Invalid );

        return this->hasPrimaryField() ? this->value( static_cast<Row>( index.row()), this->primaryFieldIndex ).toInt() : -1;
    }

    return QSqlTableModel::data( index, role );
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
QSqlField Table::field( int id ) const {
    if ( this->fields.contains( id ))
        return this->fields[id];

    return QSqlField();
}

/**
 * @brief Table::addField
 * @param id
 * @param fieldName
 * @param type
 * @param unique
 * @param autoValue
 * @param primary
 */
void Table::addField( int id, const QString &fieldName, QMetaType::Type type, bool unique, bool autoValue, bool primary ) {
    if ( this->fields.contains( id ))
        return;

    QSqlField field( fieldName, QMetaType( type ));
    field.setAutoValue( autoValue );

    if ( unique )
        this->uniqueFields << field;

    this->fields[id] = field;
    if ( primary ) {
        if ( this->hasPrimaryField()) {
            qWarning( Database_::Debug ) << Table::tr( R"(table "%1" already has a primary field")" ).arg( this->tableName());
            return;
        }

        this->primaryFieldIndex = id;
    }
}

/**
 * @brief Table::add
 * @param name
 */
Row Table::add( const QVariantList &arguments ) {  
    if ( this->fields.count() != arguments.count()) {
        qCCritical( Database_::Debug ) << Table::tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg( this->fields.count());
        return Row::Invalid;
    }

    // prepare statement
    QSqlRecord record( this->record());
    for ( int y = 0; y < arguments.count(); y++ ) {
        const QSqlField field( this->fields[y] );
        const QVariant argument( arguments.at( y ));

        if ( !field.isAutoValue()) {
            if ( argument.typeId() != field.metaType().id()) {
                qCCritical( Database_::Debug ) << Table::tr( "incompatible field type - %1 for argument %2 (%3), required - %4" ).arg( argument.typeId()).arg( y ).arg( FieldTypes.value( static_cast<QMetaType::Type>( field.metaType().id()))).arg( field.metaType().id());
                return Row::Invalid;
            }

            record.setValue( field.name(), argument );
        }
    }

    this->insertRecord( -1, record );

    if ( this->submitAll()) {
        this->database().commit();
    } else {
        this->database().rollback();
        qCCritical( Database_::Debug ) << Table::tr( R"(cannot insert record into table "%1" (reason - "%2")" ).arg( this->tableName()).arg( this->lastError().text());
        return Row::Invalid;
    }

    this->select();
    return static_cast<Row>( this->count() - 1 );
}

/**
 * @brief Table::prepare
 * @return
 */
QSqlQuery Table::prepare( bool ignore ) const {
    if ( !this->isValid())
        return QSqlQuery();

    // prepare statement
    QString statement(( ignore ? "insert or ignore into " : "insert into " ) + this->tableName() + " (" );
    QString values;
    for ( int y = 0; y < this->fields.count(); y++ ) {
        const QSqlField &field( this->fields[y] );
        if ( this->primaryFieldIndex == y )
            continue;

        const bool last = ( y == this->fields.count() - 1 );

        values.append( " :_" + field.name() + +( last ? " )" : "," ));
        statement.append( " " + field.name() + ( last ? " ) values(" + values : "," ));
    }
    QSqlQuery query;
    query.prepare( statement );

    return query;
}

/**
 * @brief Table::bind
 * @param query
 * @param arguments
 */
bool Table::bind( QSqlQuery &query, const QVariantList &arguments ) {
    if ( !this->isValid())
        return false;

    const qsizetype numFields = this->fields.count() - ( this->hasPrimaryField() ? 1 : 0 );
    if ( numFields != arguments.count()) {
        qCCritical( Database_::Debug )
            << Table::tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg(
                    this->fields.count());
        return false;
    }

    // prepare statement
    int y = 0;
    for ( const QSqlField &field : qAsConst( this->fields )) {
        if ( this->hasPrimaryField() && this->fields[this->primaryFieldIndex] == field )
            continue;

        const QVariant& argument( arguments.at( y ));

        // compare types
        if ( field.metaType() != argument.metaType()) {
            qCCritical( Database_::Debug )
                << Table::tr( "incompatible field type - %1 for argument %2 (%3), required - %4" )
                        .arg( argument.typeId()).arg( y ).arg( FieldTypes.value( static_cast<QMetaType::Type>( field.metaType().id()))).arg( field.metaType().id());
            return false;
        }

        // bind value
        query.bindValue( ":_" + field.name(), argument );

        y++;
    }

    return true;
}

/**
 * @brief Table::remove
 * @param row
 */
void Table::remove( const Row &row ) {
    if ( !this->isValid() || row == Row::Invalid )
        return;

    this->removeRow( static_cast<int>( row ));
    this->select();
}

/**
 * @brief Table::setValue
 * @param row
 * @param fieldId
 */
void Table::setValue( const Row &row, int fieldId, const QVariant &value ) {
    if ( !this->isValid() || row == Row::Invalid )
        return;

    this->setData( this->index( static_cast<int>( row ), fieldId ), value );
    this->submit();
}

/**
 * @brief Table::contains
 * @param field
 * @param value
 * @return
 */
bool Table::contains( const QSqlField &field, const QVariant &value ) const {
    int y;

    if ( !this->isValid())
        return false;

    for ( y = 0; y < this->count(); y++ ) {
        if ( this->record( y ).value( field.name()) == value )
            return true;
    }
    return false;
}

/**
 * @brief operator <<
 * @param debug
 * @param id
 * @return
 */
QDebug operator<<( QDebug debug, const Id &id ) {
    QDebugStateSaver saver( debug );
    debug.nospace() << "id:" << static_cast<int>( id );

    return debug;
}

/**
 * @brief operator <<
 * @param debug
 * @param row
 * @return
 */
QDebug operator<<( QDebug debug, const Row &row ) {
    QDebugStateSaver saver( debug );
    debug.nospace() << "row:" << static_cast<int>( row );

    return debug;
}

