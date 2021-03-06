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

/*
 * includes
 */
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
#ifdef SQLITE_CUSTOM
#include <QSqlDriver>
#include "sqlite/sqlite3.h"
#endif
#include "mainwindow.h"

/**
 * @brief Database::testPath checks if provided database path is valid and creates non-existent sub-directories
 * @param path database path
 * @return success
 */
bool Database::testPath( const QString &path ) {
    const QDir dir( QFileInfo( path ).absoluteDir());

    // reject empty paths
    if ( path.isEmpty()) {
        qCDebug( Database_::Debug ) << Database::tr( "empty database path" );
        return false;
    }

    // only accept absolute paths
    if ( !dir.isAbsolute()) {
        qCDebug( Database_::Debug ) << Database::tr( "relative or invalid database path \"%1\"" ).arg( path );
        return false;
    }

    if ( !dir.exists()) {
        qCDebug( Database_::Debug ) << Database::tr( "making non-existant database path \"%1\"" ).arg( dir.absolutePath());
        dir.mkpath( dir.absolutePath());

        if ( !dir.exists())
            return false;
    }

    return true;
}

/**
 * @brief Database::Database initializes database
 * @param parent any qobject parent
 */
Database::Database( QObject *parent ) : QObject( parent ) {
    QSqlDatabase database( QSqlDatabase::database());

    // validate path
    if ( !testPath( Variable::string( "databasePath" ))) {
        Variable::setString( "databasePath",
                                         QDir( QDir::homePath() + "/" + Main::Path ).absolutePath() + "/" +
                                         "database.db" );

        if ( !this->testPath( Variable::string( "databasePath" )))
            qFatal( QT_TR_NOOP_UTF8( "could not create database path" ) );
    }

    // failsafe
    QFile file( Variable::string( "databasePath" ));
    if ( !file.exists()) {
        if ( QFile::copy( ":/initial/database.db", Variable::string( "databasePath" ))) {
            qCDebug( Database_::Debug ) << Database::tr( "using built-in database" )
                                        << Variable::string( "databasePath" );
        } else {
            // this should never happen, but just in case
            file.open( QFile::WriteOnly );
            file.close();
            qCDebug( Database_::Debug ) << Database::tr( "creating non-existant database" )
                                        << Variable::string( "databasePath" );
        }

        if ( !file.exists())
            qFatal( QT_TR_NOOP_UTF8( "unable to create database file" ) );

        file.setPermissions( QFileDevice::ReadOwner | QFileDevice::WriteOwner );
    }

    // announce
    qCInfo( Database_::Debug ) << Database::tr( "loading database" );

    // failsafe
    if ( !QSqlDatabase::isDriverAvailable( "QSQLITE" ))
        qFatal( QT_TR_NOOP_UTF8( "sqlite not present on the system" ) );

    // set sqlite driver
    database = QSqlDatabase::addDatabase( "QSQLITE" );
    database.setHostName( "localhost" );
    database.setDatabaseName( QFileInfo( file ).absoluteFilePath());

    // set path and open
    if ( !database.open())
        qFatal( QT_TR_NOOP_UTF8( "could not load database" ));

    // load sql library
#ifdef SQLITE_CUSTOM
    QVariant handle( database.driver()->handle());
    if ( handle.isValid() && !QString::compare( handle.typeName(), "sqlite3*" )) {
        sqlite3 *libSqlite3 = *static_cast<sqlite3 **>( handle.data());

        if ( libSqlite3 != nullptr && database.isOpen() && database.isValid()) {
            // initialize sqlite
            qCWarning( Database_::Debug ) << this->tr( "initializing custom sqlite lib" );
            sqlite3_initialize();

            // localeCompare lambda
            auto localeCompare = []( void *, int l0, const void* d0, int l1, const void* d1 ) {
                return QString::localeAwareCompare(
                            QString::fromRawData( reinterpret_cast<const QChar*>( d0 ), l0 / static_cast<int>( sizeof( QChar ))),
                            QString::fromRawData( reinterpret_cast<const QChar*>( d1 ), l1 / static_cast<int>( sizeof( QChar )))) < 0;
            };

            // initialize
            if ( sqlite3_create_collation( libSqlite3, "localeCompare", SQLITE_UTF16, 0, localeCompare ) != SQLITE_OK )
                qCWarning( Database_::Debug ) << this->tr( "could not add locale aware string collation" );
        }
    }
#endif

    // done
    this->setInitialised();
}

/**
 * @brief Database::removeOrphanedEntries removes orphaned entries in database tables
 */
void Database::removeOrphanedEntries() {
    for ( Table *table : qAsConst( this->tables ))
        table->removeOrphanedEntries();
}

/**
 * @brief Database::incrementCounter
 */
void Database::incrementCounter() {
    if ( Variable::isDisabled( "backup/enabled" ))
        return;

    // increment value
    this->m_counter++;

    if ( this->count() >= Variable::integer( "backup/changes" )) {
        this->resetCounter();
        this->writeBackup();
    }
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
    qCInfo( Database_::Debug ) << Database::tr( "clearing tables" );
    for ( Table *table : qAsConst( this->tables ))
        table->clear();

    // delete all tables
    qDeleteAll( this->tables );

    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase database( QSqlDatabase::database());
        if ( database.isOpen()) {
            open = true;
            connectionName = database.connectionName();

            qCInfo( Database_::Debug ) << Database::tr( "closing database" );
            database.close();
        }
    }

    // only now we can sever the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );

#ifdef SQLITE_CUSTOM
    // shutdown
    sqlite3_shutdown();
#endif
}

/**
 * @brief Database::add adds and validates Table instance to database
 * @param table Table instance (QSqlTableModel)
 */
bool Database::add( Table *table ) {
    QSqlDatabase database( QSqlDatabase::database());
    const QStringList tableList( database.tables());

    // store table
    this->tables[table->tableName()] = table;

    // announce
    if ( !tableList.count())
        qCInfo( Database_::Debug ) << Database::tr( "creating an empty database" );

    // validate schema
    bool found = false;
    for ( const QString &tableName : tableList ) {
        if ( !QString::compare( table->tableName(), tableName )) {
            for ( const Field &field : qAsConst( table->fields )) {

                if ( !database.record( table->tableName()).contains( field->name())) {
                    qCCritical( Database_::Debug )
                        << Database::tr( R"(database field mismatch in table "%1", field - "%2")" ).arg( tableName, field->name());
                    return false;
                }

                // ignore unsigned ints for now
                const QVariant::Type internalType = field->type() == QVariant::UInt ? QVariant::Int : field->type();
                const QVariant::Type databaseType = database.record( table->tableName()).field( field->id()).type();

                if ( internalType != databaseType ) {
                    qCCritical( Database_::Debug )
                        << Database::tr( R"(database type mismatch in table "%1", field - "%2")" ).arg(
                                tableName, field->name());
                    return false;
                }
            }
            found = true;
        }
    }

    QString statement;
    QSqlQuery query;

    if ( !found ) {
        // announce
        qCInfo( Database_::Debug ) << Database::tr( "creating an empty table - \"%1\"" ).arg( table->tableName());

        // prepare statement
        for ( const Field &field : qAsConst( table->fields )) {
            statement.append( QString( "%1 %2" ).arg( field->name(), field->format()));

            if ( field->isUnique())
                statement.append( " unique" );

            if ( QString::compare( field->name(), table->fields.last()->name()))
                statement.append( ", " );
        }

        // check for constraints
        QString constraints;
        const int tc = table->constraints.count();

        if ( tc > 0 ) {
            for ( int y = 0; y < table->constraints.count(); y++ ) {
                constraints.append( "unique( " );

                const int cc = table->constraints.at( y ).count();
                for ( int k = 0; k < cc; k++ ) {
                    const QSharedPointer<Field_> field( table->constraints.at( y ).at( k ));
                    constraints.append( field->name());
                    constraints.append( k == cc - 1 ? " )" : ", " );

                }

                if ( y < tc - 1 )
                    constraints.append( ", " );
            }

            statement.append( ", " );
            statement.append( constraints );
        }

        if ( !query.exec( QString( "create table if not exists %1 ( %2 )" ).arg( table->tableName(), statement )))
            qCCritical( Database_::Debug )
                << Database::tr( R"(could not create table - "%1", reason - "%2")" ).arg( table->tableName(), query.lastError().text());
    }

    // table has been verified and is marked as valid
    table->setValid();

    // create table model
    table->setTable( table->tableName());

    // load data
    if ( !table->select()) {
        qCCritical( Database_::Debug )
            << Database::tr( "could not initialize model for table - \"%1\"" ).arg( table->tableName());
        table->setValid( false );
    }

    return true;
}

/**
 * @brief Database::writeBackup
 */
void Database::writeBackup() {
    const QFileInfo info( Variable::string( "databasePath" ));
    const QDir dir( info.absolutePath() + + "/backups/" );

    if ( !dir.exists()) {
        dir.mkpath( dir.absolutePath());
        qCDebug( Database_::Debug ) << this->tr( "making non-existant database backup path \"%1\"" ).arg( dir.absolutePath());

        if ( !dir.exists())
            qFatal( QT_TR_NOOP_UTF8( "could not create database backup path" ));
    }

    // backup database filename
    const QString backup( QString( "%1/%2_%3.db" )
                          .arg( dir.absolutePath(),
                                info.fileName().remove( ".db" ),
                                QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));

    // announce
    qCDebug( Database_::Debug ) << this->tr( "performing backup to \"%1\"" ).arg( backup );

    // perform a simple copy
    QFile::copy( Variable::string( "databasePath" ),
                 QString( "%1/%2_%3.db" )
                 .arg( dir.absolutePath(),
                       info.fileName().remove( ".db" ),
                       QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
}

/**
 * @brief Database::attach
 * @param path
 */
void Database::attach( const QFileInfo &info ) {
    // get current event if any
    const Row row = MainWindow::instance()->currentEvent();
    if ( row == Row::Invalid )
        return;

    // get current event name
    const QString currentTitle( Event::instance()->title( row ));

    // check if database exists
    if ( !info.exists()) {
        qCDebug( Database_::Debug ) << this->tr( "database \"%1\" does not exist" ).arg( info.fileName());
        return;
    }

    // attach a foreign database
    QSqlQuery query;
    if ( !query.exec( QString( "attach '%1' as merge" ).arg( info.absoluteFilePath()))) {
        qCritical( Database_::Debug ) << this->tr( "could not attach database, reason - \"%1\"" ).arg( query.lastError().text());
        return;
    }

    // check if tasks match in both tables
    // TODO: also check API
    if ( !query.exec( QString( "select not exists ( select * from %1 except select * from merge.%1 ) and not exists ( select * from merge.%1 except select * from %1 )" ).arg( Task::instance()->tableName()))) {
        qCritical( Database_::Debug ) << this->tr( "could not compare task tables" );
        return;
    } else {
        bool result = false;
        if ( query.next())
            result = query.value( 0 ).toBool();

        // abort in case of mismatch
        if ( !result ) {
            qCritical( Database_::Debug ) << this->tr( "task table mismatch" );
            return;
        }
    }

    // find a matching event in the foreign database
    bool found = false;
    Id eventId = Id::Invalid;
    if ( query.exec( QString( "select * from merge.%1" ).arg( Event::instance()->tableName()))) {
        while ( query.next()) {
            const QString title( query.record().value( Event::Title ).toString());
            eventId = static_cast<Id>( query.record().value( Event::ID ).toInt());

            if ( eventId != Id::Invalid && !QString::compare( title, currentTitle )) {
                found = true;
                break;
            }
        }
    }

    // abort if a matching event is not found
    if ( !found ) {
        qCritical( Database_::Debug ) << this->tr( "could not find matching event" );
        return;
    }

    // highest id lambda
    auto getHiId = []( const QString &table, const QString &fieldName ) {
        QSqlQuery query;

        query.exec( QString( "select max( %1 ) from %2" ).arg( fieldName, table ));
        if ( query.next())
            return query.value( 0 ).toInt();

        return static_cast<int>( Id::Invalid );
    };

    // get highest combo id from current database
    int comboHiId = getHiId( Log::instance()->tableName(),  Log::instance()->fieldName( Log::Combo ));
    int teams = 0, logs = 0;

    // find unique teams that are not in the current event
    if ( query.exec( QString( "select * from merge.%1 where %2 not in ( select %2 from %1 )" )
                     .arg( Team::instance()->tableName(),
                           Team::instance()->fieldName( Team::Title )))) {

        // go through the team list
        while ( query.next()) {
            const QString title( query.record().value( Team::Title ).toString());
            const Id teamId = static_cast<Id>( query.record().value( Team::ID ).toInt());
            const int members = query.record().value( Team::Members ).toInt();
            const QTime finish( QTime::fromString( query.record().value( Team::Finish ).toString(), Database_::TimeFormat ));
            const QString reviewer( query.record().value( Team::Reviewer ).toString());

            if ( teamId == Id::Invalid )
                continue;

            // add a new team
            const Row row = Team::instance()->add( title, members, finish, reviewer );
            if ( row == Row::Invalid )
                return;

            const Id newTeamId = Team::instance()->id( row );
            if ( newTeamId == Id::Invalid )
                return;

            // find logs that belong to the current team
            QSqlQuery subQuery;
            QMap<Id, Id> comboIdRemap;
            if ( subQuery.exec( QString( "select * from merge.%1 where %2=%3" )
                                .arg( Log::instance()->tableName(),
                                      Log::instance()->fieldName( Log::Team ),
                                      QString::number( static_cast<int>( teamId ))))) {

                // go through the log list
                while ( subQuery.next()) {
                    const Id taskId = static_cast<Id>( subQuery.record().value( Log::Task ).toInt());
                    const int multi = subQuery.record().value( Log::Multi ).toInt();
                    const Id comboId = static_cast<Id>( subQuery.record().value( Log::Combo ).toInt());

                    // remap comboIds
                    if ( !comboIdRemap.contains( comboId ))
                        comboIdRemap[comboId] = comboId == Id::Invalid ? Id::Invalid : static_cast<Id>( comboHiId++ );

                    // add logs
                    Log::instance()->add( taskId, newTeamId, multi, comboIdRemap[comboId] );
                    logs++;
                }
            }

            teams++;
        }
    } else {
        qCritical( Database_::Debug ) << this->tr( "could not perform team query" );
        return;
    }

    // report
    qCDebug( Database_::Debug ) << this->tr( "imported %1 teams and %2 logs" ).arg( teams ).arg( logs );

    // detach database
    query.exec( "detach merge" );
}
