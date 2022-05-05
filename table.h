/*
 * Copyright (C) 2018-2019 Armands Aleksejevs
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
#include <QSqlRelationalTableModel>
#include <QSharedPointer>
#include <QSqlRecord>
#include <QSqlField>

//
// classes
//
class QDebug;
class QSqlQuery;

/**
 * namespace Database
 */
namespace Database_ {
[[maybe_unused]] static constexpr const char *TimeFormat = "hh:mm";
}

/**
 * @brief The Id enum strong-typed id
 */
enum class Id : int {
    Invalid = -1
};
QDebug operator<<( QDebug debug, const Id &id );
Q_DECLARE_METATYPE( Id )

/**
 * @brief The Row enum strong-typed row
 */
enum class Row : int {
    Invalid = -1
};
QDebug operator<<( QDebug debug, const Row &row );

Q_DECLARE_METATYPE( Row )

/*
 * FIELD macro generates a lowercase fieldName from field index (enum)
 */
// field map

#define FIELD( fieldId, type ) this->addField( fieldId, QString( #fieldId ).toLower(), QMetaType::type )
#define UNIQUE_FIELD( fieldId, type ) this->addField( fieldId, QString( #fieldId ).toLower(), QMetaType::type, true )
#define PRIMARY_FIELD( fieldId ) this->addField( fieldId, QString( #fieldId ).toLower(), QMetaType::Int, true, true, true )
#define FIELD_GETTER( type, fieldId, name ) public: [[nodiscard]] type name( const Row &row ) const { return this->value( row, fieldId ).value<type>(); } type name( const Id &id ) const { return this->value( id, fieldId ).value<type>(); }
#define FIELD_SETTER( type, fieldId ) public slots: void set##fieldId( const Row &row, const type &variable ) { this->setValue( row, fieldId, QVariant::fromValue( variable )); }
#define INITIALIZE_FIELD( type, fieldId, name ) FIELD_GETTER( type, fieldId, name ) FIELD_SETTER( type, fieldId )

/**
 * @brief The Table_ class
 */
class Table : public QSqlTableModel {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( Table )
    friend class Database;

public:
    /**
     * @brief The Roles enum
     */
    enum Roles {
        IDRole = Qt::UserRole
    };
    Q_ENUM( Roles )

    explicit Table( const QString &tableName = QString()) { this->setTable( tableName ); }
    ~Table() override {
        this->setValid( false );
        this->clear();
    }

    /**
     * @brief isValid
     * @return
     */
    [[nodiscard]] bool isValid() const { return this->m_valid; }

    /**
     * @brief hasPrimaryField
     * @return
     */
    [[nodiscard]] bool hasPrimaryField() const { return this->primaryFieldIndex != -1; }
    Q_INVOKABLE [[nodiscard]] int count() const;

    [[nodiscard]] virtual QVariant value( const Row &row, int fieldId ) const;
    [[nodiscard]] virtual QVariant value( const Id &id, int fieldId ) const;

    /**
     * @brief contains
     * @param fieldId
     * @param value
     * @return
     */
    [[nodiscard]] bool contains( int fieldId, const QVariant &value ) const { return this->contains( this->field( fieldId ), value ); }
    bool select() override;
    [[nodiscard]] QVariant data( const QModelIndex &index, int role ) const override;
    void setFilter( const QString &filter ) override;

    /**
     * @brief fieldName
     * @param id
     * @return
     */
    [[nodiscard]] QString fieldName( int id ) const { return this->field( id ).name(); }

    /**
     * @brief row
     * @param index
     * @return
     */
    [[nodiscard]] Row row( const int index ) const {
        if ( index < 0 || index >= this->count()) return Row::Invalid;
        return static_cast<Row>( index );
    }

    /**
     * @brief row
     * @param index
     * @return
     */
    [[nodiscard]] Row row( const QModelIndex &index ) const {
        if ( index.row() < 0 || index.row() >= this->count() || index.model() != this )
            return Row::Invalid;

        return static_cast<Row>( index.row());
    }
    [[nodiscard]] Row row( const Id &id ) const;

    /**
     * @brief addUniqueConstraint
     * @param constrainedFields
     */
    [[maybe_unused]] void addUniqueConstraint( const QList<QSqlField> &constrainedFields ) { this->constraints << constrainedFields; }
    [[maybe_unused]][[nodiscard]] QSqlQuery prepare( bool ignore = true ) const;
    [[maybe_unused]] bool bind( QSqlQuery &query, const QVariantList &arguments );

public slots:
    /**
     * @brief setValid
     * @param valid
     */
    void setValid( bool valid = true ) { this->m_valid = valid; }
    void addField( int id, const QString &fieldName = QString(), QMetaType::Type type = QMetaType::UnknownType, bool unique = false, bool autoValue = false, bool primary = false );
    Row add( const QVariantList &arguments );
    virtual void remove( const Row &row );
    void setValue( const Row &row, int fieldId, const QVariant &value );

    /**
     * @brief removeOrphanedEntries
     */
    virtual void removeOrphanedEntries() {}

protected:
    QMap<int, QSqlField> fields;
    QList<QSqlField> uniqueFields;
    [[nodiscard]] QSqlField field( int id ) const;
    [[nodiscard]] bool contains( const QSqlField &field, const QVariant &value ) const;

private:
    bool m_valid = false;
    QList<QList<QSqlField>> constraints;
    int primaryFieldIndex = -1;

    static QMultiMap<QMetaType::Type, QString> FieldTypes;
};

// declare enums
Q_DECLARE_METATYPE( Table::Roles )
