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
    query.exec( QString( "SELECT %1, %2 from %3 where %1=%4" )
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
    return this->record().field( id );
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
void Table::appendField( const QString &fieldName, QMetaType::Type type, bool unique, bool autoValue, bool primary ) {
    if ( !this->database().isOpen() || this->tableName().isEmpty()) {
        qWarning( Database_::Debug ) << Table::tr( R"(database not loaded)" );
        return;
    }

    if ( !FieldTypes.contains( type )) {
        qWarning( Database_::Debug ) << Table::tr( R"(unsupported field type %1)" ).arg( type );
        return;
    }

    if ( primary && this->hasPrimaryField()) {
        qWarning( Database_::Debug ) << Table::tr( R"(table "%1" already has a primary field")" ).arg( this->tableName());
        return;
    }

    QSqlField field;
    if ( this->record().contains( fieldName )) {
        field = this->record().field( fieldName );

        if ( primary )
            this->primaryFieldIndex = this->record().indexOf( fieldName );
    } else {
        field = QSqlField( fieldName, QMetaType( type ));
        field.setAutoValue( autoValue );
        this->record().append( field );

        if ( primary )
            this->primaryFieldIndex = static_cast<int>( this->tmpFields.count());

        this->tmpFields << field;
    }

    if ( unique )
        this->uniqueFields << field.name();

    this->fields++;
}

/**
 * @brief Table::add
 * @param name
 */
Row Table::add( const QVariantList &arguments ) {  
    if ( this->fields != arguments.count()) {
        qCCritical( Database_::Debug ) << Table::tr( "argument count mismatch - %1, required - %2" ).arg( arguments.count()).arg( this->record().count());
        return Row::Invalid;
    }

    // record might have extraneous fields when custom select statement is set
    // these have to be removed, otherwise record will not be inserted
    QSqlRecord record( this->record());
    const int remove = static_cast<int>( record.count() - this->fields );
    for ( int y = 0; y < remove; y++ )
        record.remove( record.count() - 1 );

    // prepare statement
    for ( int y = 0; y < arguments.count(); y++ ) {
        const QSqlField field( this->record().field( y ));
        const QVariant argument( arguments.at( y ));

        if ( argument.typeId() != field.metaType().id()) {
            qCCritical( Database_::Debug ) << Table::tr( "incompatible field type - %1 for argument %2 (%3), required - %4" ).arg( argument.typeId()).arg( y ).arg( FieldTypes.value( static_cast<QMetaType::Type>( field.metaType().id()))).arg( field.metaType().id());
            return Row::Invalid;
        }

        if ( y != this->primaryFieldIndex )
            record.setValue( field.name(), field.isAutoValue() ? 0 : argument );
    }

    if ( !this->insertRecord( -1, record )) {
        qCCritical( Database_::Debug ) << Table::tr( R"(cannot insert record into table "%1" (reason - "%2")" ).arg( this->tableName()).arg( this->lastError().text());

        return Row::Invalid;
    }

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
    for ( int y = 0; y < this->record().count(); y++ ) {
        const QSqlField &field( this->record().field( y ));
        if ( this->primaryFieldIndex == y )
            continue;

        const bool last = ( y == this->record().count() - 1 );

        values.append( " :_" + field.name() + +( last ? " )" : "," ));
        statement.append( " " + field.name() + ( last ? " ) values(" + values : "," ));
    }
    QSqlQuery query;
    query.prepare( statement );

    return query;
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

