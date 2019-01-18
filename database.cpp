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
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QApplication>
#include "database.h"
#include "table.h"
#include "field.h"
#include "main.h"
#include "variable.h"
#include "log.h"
#include "team.h"
#include "task.h"
#include "event.h"

/**
 * @brief Database::testPath
 * @param path
 * @return
 */
bool Database::testPath( const QString &path ) {
    const QDir dir( QFileInfo( path ).absoluteDir());

    // reject empty paths
    if ( path.isEmpty()) {
        qCDebug( Database_::Debug ) << this->tr( "empty database path" );
        return false;
    }

    // only accept absolute paths
    if ( !dir.isAbsolute()) {
        qCDebug( Database_::Debug ) << this->tr( "relative or invalid database path \"%1\"" ).arg( path );
        return false;
    }

    if ( !dir.exists()) {
        qCDebug( Database_::Debug ) << this->tr( "making non-existant database path \"%1\"" ).arg( dir.absolutePath());
        dir.mkpath( dir.absolutePath());

        if ( !dir.exists())
            return false;
    }

    return true;
}

/**
 * @brief Database::Database
 * @param parent
 */
Database::Database( QObject *parent ) : QObject( parent ), m_initialised( false ) {
    QSqlDatabase database( QSqlDatabase::database());

    if ( !testPath( Variable::instance()->string( "databasePath" ))) {
        Variable::instance()->setString( "databasePath", QDir( QDir::homePath() + "/" + Main::Path ).absolutePath() + "/" + "database.db" );

        if ( this->testPath( Variable::instance()->string( "databasePath" )))
            qFatal( QT_TR_NOOP_UTF8( "could not create database path" ));
    }

    // failsafe
    QFile file( Variable::instance()->string( "databasePath" ));
    if ( !file.exists()) {
        file.open( QFile::WriteOnly );
        file.close();
        qCDebug( Database_::Debug ) << this->tr( "creating non-existant database" );

        if ( !file.exists())
            qFatal( QT_TR_NOOP_UTF8( "unable to create database file" ));
    }

    // announce
    qCInfo( Database_::Debug ) << this->tr( "loading database" );

    // failsafe
    if ( !database.isDriverAvailable( "QSQLITE" ))
        qFatal( QT_TR_NOOP_UTF8( "sqlite not present on the system" ));

    // set sqlite driver
    database = QSqlDatabase::addDatabase( "QSQLITE" );
    database.setHostName( "localhost" );
    database.setDatabaseName( QFileInfo( file ).absoluteFilePath());

    // set path and open
    if ( !database.open())
        qFatal( QT_TR_NOOP_UTF8( "could not load database" ));

    // done
    this->setInitialised();
}

/**
 * @brief Database::removeOrphanedEntries
 */
void Database::removeOrphanedEntries() {
    foreach ( Table *table, this->tables )
        table->removeOrphanedEntries();
}

/**
 * @brief Database::~Database
 */
Database::~Database() {
    QString connectionName;
    bool open = false;

    // remove orphans
    this->removeOrphanedEntries();

    // announce
    qCInfo( Database_::Debug ) << this->tr( "unloading database" );
    this->setInitialised( false );

    // unbind variables
    Variable::instance()->unbind( "eventId" );
    Variable::instance()->unbind( "teamId" );
    qCInfo( Database_::Debug ) << this->tr( "clearing tables" );
    foreach ( Table *table, this->tables )
        table->clear();

    qDeleteAll( this->tables );

    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase database( QSqlDatabase::database());
        if ( database.isOpen()) {
            open = true;
            connectionName = database.connectionName();

            qCInfo( Database_::Debug ) << this->tr( "closing database" );
            database.close();
        }
    }

    // only now we can sever the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}

/**
 * @brief Database::add
 * @param table
 */
void Database::add( Table *table ) {
    QSqlDatabase database( QSqlDatabase::database());
    const QStringList tables( database.tables());
    QString statement;
    QSqlQuery query;
    bool found = false;

    // store table
    this->tables[table->tableName()] = table;

    // announce
    if ( !tables.count())
        qCInfo( Database_::Debug ) << this->tr( "creating an empty database" );

    // validate schema
    foreach ( const QString &tableName, tables ) {
        if ( !QString::compare( table->tableName(), tableName )) {
            foreach ( const Field &field, qAsConst( table->fields )) {
                if ( !database.record( table->tableName()).contains( field->name())) {
                    qCCritical( Database_::Debug ) << this->tr( "database field mismatch" );
                    return;
                }
            }
            found = true;
        }
    }

    if ( !found ) {
        // announce
        qCInfo( Database_::Debug ) << this->tr( "creating an empty table - \"%1\"" ).arg( table->tableName());

        // prepare statement
        foreach ( const Field &field, qAsConst( table->fields )) {
            statement.append( QString( "%1 %2" ).arg( field->name()).arg( field->format()));

            if ( QString::compare( field->name(), table->fields.last()->name()))
                statement.append( ", " );
        }

        if ( !query.exec( QString( "create table if not exists %1 ( %2 )" ).arg( table->tableName()).arg( statement )))
            qCCritical( Database_::Debug ) << this->tr( "could not create table - \"%1\", reason - \"%2\"" ).arg( table->tableName()).arg( query.lastError().text());
    }

    // table has been verified and is marked as valid
    table->setValid();

    // create table model
    table->setTable( table->tableName());

    // load data
    if ( !table->select()) {
        qCCritical( Database_::Debug ) << this->tr( "could not initialize model for table - \"%1\"" ).arg( table->tableName());
        table->setValid( false );
    }
}
