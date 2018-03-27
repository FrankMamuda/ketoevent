/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include "main.h"
#include <QDir>
#include <QCryptographicHash>
#include <QSqlDatabase>
#include <QSqlField>
#include <QMessageBox>
#include "mainwindow.h"

/**
 * @brief Database::makePath makes a filesystem path if non existant
 * @param path filesystem path
 */
void Database::makePath( const QString &path ) {
    QString fullPath;

    // default path?
    if ( path.isEmpty()) {
#ifdef Q_OS_UNIX
        fullPath = QString( QDir::homePath() + "/.ketoevent/" );
#else
        fullPath = QString( QDir::currentPath() + "/" );
#endif
        fullPath.append( "/" );
        fullPath.append( KetoEvent::defaultDatabase );
    } else
        fullPath = path;

    // make path id nonexistant
    QFileInfo db( fullPath );
    QDir dir;
    dir.setPath( db.absolutePath());

    if ( !dir.exists()) {
        dir.mkpath( db.absolutePath());
        if ( !dir.exists())
            Common::error( CLFatalError, QObject::tr( "could not create database path - \"%1\"\n" ).arg( fullPath ));
    }

    // store path
    Main::instance()->path = fullPath;
    Main::instance()->path.replace( "//", "/" );
}

/**
 * @brief Database::highestId returns highest id in database either for combo, task, etc.
 * @param type entry type
 * @return id
 */
int Database::highestId( IdTypes type ) {
    int id = 1;

    switch ( type ) {
    case ComboId:
        foreach ( Log *log, Main::instance()->logList ) {
            if ( log->comboId() > id )
                id = log->id();
        }
        break;

    case TeamId:
        foreach ( Team *team, Main::instance()->teamList ) {
            if ( team->id() > id )
                id = team->id();
        }
        break;

    case LogId:
        foreach ( Log *log, Main::instance()->logList ) {
            if ( log->id() > id )
                id = log->id();
        }
        break;

    case TaskId:
        foreach ( Task *task, Main::instance()->taskList ) {
            if ( task->id() > id )
                id = task->id();
        }
        break;


    case NoId:
    default:
        break;
    }
    return id;
}

/**
 * @brief Database::stringToHash returns md5 of a string
 * @param input text string
 * @return md5 of the given string
 */
QString Database::stringToHash( const QString &input ) {
    QCryptographicHash *hash = new QCryptographicHash( QCryptographicHash::Md5 );
    hash->addData( input.toLatin1().constData(), input.length());
    return QString( hash->result().toHex().constData());
}

/**
 * @brief Database::attach attaches a database to be imported
 * @param path database path in filesystem
 * @param import import options (logs, teams, etc.)
 */
void Database::attach( const QString &path, Import import ) {
    QSqlQuery query;
    QString dbPath = path + "import";
    QStringList tables;
    int eventId = -1;
    int mismatch = 0;

    // announce
    Common::print( CLMsg + QObject::tr( "attaching database %1\n" ).arg( path ), Common::DatabaseDebug );

    // write backup just in case
    Database::writeBackup();

    // check database
    QFile::copy( path, dbPath );
    QFile database( dbPath );
    QFileInfo dbInfo( database );
    if ( !database.exists()) {
        Common::error( CLSoftError, QObject::tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // attach the new database
    if ( !query.exec( QString( "attach '%1' as merge" ).arg( dbPath ))) {
        Common::error( CLSoftError, QObject::tr( "could not attach database, reason - \"%1\"\n" ).arg( query.lastError().text()));
        return;
    }

    if ( query.exec( QString( "select * from merge.events where name=\"%1\"" ).arg( Event::active()->name()))) {
        while ( query.next())
            eventId = query.record().value( "id" ).toInt();
    } else {
        Common::error( CLSoftError, QObject::tr( "database \"%1\" does not contain events\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // failsafe
    if ( eventId < 1 ) {
        Common::error( CLSoftError, QObject::tr( "database \"%1\" does not contain event \"%2\"\n" ).arg( dbInfo.fileName()).arg( Event::active()->name()));
        goto removeDB;
    }

    // get rid of junk
    query.exec( QString( "delete from merge.events where id!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.teams where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.logs where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.tasks where eventId!=%1" ).arg( eventId ));

    if ( import == LogImport ) {
        // integrity check
        query.exec( "select name, points, multi, type from merge.tasks except select name, points, multi, type from tasks;" );
        while ( query.next())
            mismatch++;

        if ( mismatch > 0 ) {
            Common::error( CLSoftError, QObject::tr( "incompatible task list in \"%1\"\n" ).arg( dbInfo.fileName()));
            goto removeDB;
        }

        // skip duplicate teams
        query.exec( QString( "delete from merge.teams where name in (select name from teams)" ));

        // skip orphaned logs
        query.exec( QString( "delete from merge.logs where teamId not in (select id from merge.teams)" ));
    } else if ( import == TaskImport ) {
        // skip duplicate tasks
        query.exec( QString( "delete from merge.tasks where name in (select name from tasks)" ));
    }

    // update connections
    if ( import == TaskImport ) {
        query.exec( QString( "update merge.tasks set eventId=%1" ).arg( Event::active()->id()));
        query.exec( "drop table merge.events" );
        query.exec( "drop table merge.logs" );
        query.exec( "drop table merge.teams" );
        tables << "tasks";
    }

    if ( import == LogImport ) {
        query.exec( QString( "update merge.teams set eventId=%1" ).arg( Event::active()->id()));
        query.exec( "update merge.teams set id=-id" );
        query.exec( QString( "update merge.teams set id=%1-id" ).arg( Database::highestId( TeamId )));
        query.exec( "update merge.logs set id=-id" );
        query.exec( QString( "update merge.logs set id=%1-id" ).arg( Database::highestId( LogId )));
        query.exec( QString( "update merge.logs set teamId=teamId+%1" ).arg( Database::highestId( TeamId )));
        query.exec( QString( "update merge.logs set comboId=comboId+%1 where comboId!=-1" ).arg( Database::highestId( LogId )));
        query.exec( "drop table merge.tasks" );
        query.exec( "drop table merge.events" );
        tables << "teams" << "logs";
    }

    // compare task hashes
    unsigned int y, k;

    // go through all tables and all fields
    for ( y = 0; y < API::numTables; y++ ) {
        table_t api = API::tables[y];
        QStringList fields;

        if ( tables.contains( api.name, Qt::CaseInsensitive )) {
            for ( k = 0; k < api.numFields; k++ ) {
                tableField_t apif = api.fields[k];

                // generate a list of matching fields
                fields << apif.name;
            }

            if ( !query.exec( QString( "insert into %1 ( %2 ) select %2 from merge.%1" ).arg( api.name ).arg( fields.join( ", " )))) {
                Common::error( CLSoftError, QObject::tr( "could not perform import, reason \"%1\"\n" ).arg( query.lastError().text()));
                break;
            }
        }
    }

removeDB:
    query.exec( "detach merge" );
    database.close();
    database.remove();

    // refresh gui
    Main::instance()->clearEvent();
    Database::reload( Variable::instance()->string( "databasePath" ));
    MainWindow *mainWindow = qobject_cast<MainWindow *>( Main::instance()->parent());
    if ( mainWindow == nullptr )
        return;

    mainWindow->fillTasks();
    mainWindow->selectTeam();
}

/**
 * @brief Database::createStructure creates an empty, structured database
 * @param prefix import prefix
 */
bool Database::createStructure( const QString &prefix ) {
    unsigned int y, k, l, n;
    QSqlQuery query;

    // failafe
    QFile dbFile( Variable::instance()->string( "databasePath" ));
    if ( !dbFile.exists()) {
        Common::error( CLFatalError, QObject::tr( "unable to create database file\n" ));
        return false;
    }

    // additional structure check (API9)
    QSqlDatabase db = QSqlDatabase::database();
    QStringList tables = db.tables();
    bool mismatch = false;

    // announce
    if ( !tables.count())
        Common::print( CLMsg + QObject::tr( "creating an empty database '%1'\n" ).arg( Variable::instance()->string( "databasePath" )), Common::DatabaseDebug );

    // TODO: api mismatch should also be handled here (reading a single field from events table)
    //       also schema mismatch could be handled here
    // NOTE: since only two APIs (API0/2013 and API9/2013-2016) are actually used, this greatly simplifies importing
    for ( y = 0; y < API::numTables; y++ ) {
        table_t table = API::tables[y];

        foreach ( QString tableName, tables ) {
            if ( !QString::compare( table.name, tableName, Qt::CaseInsensitive )) {
                for ( k = 0; k < table.numFields; k++ ) {
                    if ( !db.record( table.name ).contains( table.fields[k].name ))
                        mismatch = true;
                }
            }
        }
    }

    // check if database can be imported
    if ( mismatch && tables.count() > 0 ) {
        mismatch = false;

        // announce
        Common::print( CLMsg + QObject::tr( "table field mismatch in '%1'. different API?\n" ).arg( Variable::instance()->string( "databasePath" )), Common::DatabaseDebug );

        // check if table structure matches API0
        for ( y = 0; y < API0::numTables; y++ ) {
            table_t table = API0::tables[y];

            foreach ( QString tableName, tables ) {
                if ( !QString::compare( table.name, tableName, Qt::CaseInsensitive )) {
                    for ( k = 0; k < table.numFields; k++ ) {
                        if ( !db.record( table.name ).contains( table.fields[k].name ))
                            mismatch = true;
                    }
                }
            }
        }

        // attempt to import API9
        if ( !mismatch ) {
            QMessageBox msgBox;
            int state;

            msgBox.setText( QObject::tr( "Attempt to import API0 (2013) database?" ));
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msgBox.setDefaultButton( QMessageBox::Yes );
            msgBox.setIcon( QMessageBox::Warning );
            state = msgBox.exec();

            // check options
            switch ( state ) {
            case QMessageBox::Yes:
            {
                int teamId, sum, comboTaskId;
                QList <int>teamIdList;

                // create initial table structure (temporary)
                if ( !Database::createEmptyTable( "tmp_" ))
                    return false;

                // go through all tables and all fields
                for ( y = 0; y < API::numTables; y++ ) {
                    table_t api = API::tables[y];

                    for ( k = 0; k < API0::numTables; k++ ) {
                        table_t api0 = API0::tables[k];

                        if ( !QString::compare( api.name, api0.name, Qt::CaseInsensitive )) {
                            QStringList fields;

                            for ( l = 0; l < api.numFields; l++ ) {
                                tableField_t apif = api.fields[l];

                                for ( n = 0; n < api0.numFields; n++ ) {
                                    tableField_t api0f = api0.fields[n];

                                    // generate a list of matching fields
                                    if ( !QString::compare( apif.name, api0f.name, Qt::CaseInsensitive ))
                                        fields << apif.name;
                                }
                            }

                            // copy data (tested only for API0)
                            if ( !QString::compare( api.name, "teams", Qt::CaseInsensitive ))
                                query.exec( QString( "insert into tmp_%1 ( %2, finishTime ) select %2, finish from %1" ).arg( api.name ).arg( fields.join( ", " )));
                            else
                                query.exec( QString( "insert into tmp_%1 ( %2 ) select %2 from %1" ).arg( api.name ).arg( fields.join( ", " )));
                        }
                    }
                }

                // add a combo task
                if ( !query.exec( QObject::tr( "insert into tmp_tasks values ( null, '%1', 1, 999, %2, %3, nullptr, 1, '%4' )" )
                                 .arg( KetoEvent::comboString )
                                 .arg( static_cast<int>( Task::Bold ))
                                 .arg( static_cast<int>( Task::Multi ))
                                 .arg( KetoEvent::comboDescription )
                                  )) {
                    Common::error( CLSoftError, QObject::tr( "could not add combo task\n" ));
                    return false;
                }

                // get combo task id
                comboTaskId = query.lastInsertId().toInt();
                if ( comboTaskId < 1 ) {
                    Common::error( CLSoftError, QObject::tr( "bad combo task id\n" ));
                    return false;
                }

                // build a teamId list
                query.exec( "select * from teams" );
                while ( query.next())
                    teamIdList << query.record().value( "id" ).toInt();

                // go through all teams and add up combos
                foreach ( teamId, teamIdList ) {
                    query.exec( QString( "select sum( combo ) from logs where teamId=%1" ).arg( teamId ));
                    while ( query.next())
                        sum = query.value( 0 ).toInt();

                    // insert log
                    query.exec( QString( "insert into tmp_logs values ( null, %1, %2, %3, -1 )" )
                                .arg( sum )
                                .arg( comboTaskId )
                                .arg( teamId ));
                }

                // go through all tables and all fields
                for ( y = 0; y < API::numTables; y++ ) {
                    table_t api = API::tables[y];

                    // drop legacy tables, rename temporary tables to permanent
                    query.exec( QString( "drop table %1" ).arg( api.name ));
                    query.exec( QString( "alter table tmp_%1 rename to %1" ).arg( api.name ));
                }

                // add a single event
                Event::add( "Imported 2013 event" );

                // fill nullptr values with blank data
                query.exec( "update teams set lock=0" );
                query.exec( "update teams set reviewer='imported'" );
                query.exec( "update teams set eventId=1" );
                query.exec( "update tasks set eventId=1" );
                query.exec( "update tasks set description=''" );
                query.exec( "update logs set comboId=-1" );
                return true;
            }
                break;

            case QMessageBox::No:
            default:
                ;
            }
        } else
            Common::error( CLFatalError, QObject::tr( "Could not import database\n" ));

        return false;
    }

    // create initial table structure (if non-existant)
    return Database::createEmptyTable( prefix );
}

/**
 * @brief Database::generateSchemas
 * @param tableName
 * @param prefix
 * @return
 */
QStringList Database::generateSchemas( const QString &prefix ) {
    unsigned int y, k;
    QStringList schemas;

    for ( y = 0; y < API::numTables; y++ ) {
        table_t api = API::tables[y];
        QString schema = QString( "create table if not exists %1%2 ( " ).arg( prefix ).arg( api.name );

        for ( k = 0; k < api.numFields; k++ ) {
            tableField_t apif = api.fields[k];
            schema.append( QString( "%1 %2" ).arg( apif.name ).arg( apif.type ));

            if ( k == api.numFields - 1 )
                schema.append( " )" );
            else
                schema.append( " ," );
        }
        schemas << schema;
    }
    return schemas;
}

/**
 * @brief Database::createEmptyTable
 * @param prefix
 * @param suffix
 * @return
 */
bool Database::createEmptyTable( const QString &prefix ) {
    QSqlQuery query;
    QStringList schemas;

    // get schemas
    schemas = Database::generateSchemas( prefix );
    foreach ( QString schema, schemas ) {
        if ( !query.exec( schema )) {
            Common::error( CLFatalError, QObject::tr( "could not create internal database structure, reason - \"%1\"\n" ).arg( query.lastError().text()));
            return false;
        }
    }
    return true;
}

/**
 * @brief Database::load reads sqlite database from filesystem
 */
bool Database::load() {
    // create database
    QFile database( Main::instance()->path );
    QFileInfo dbInfo( database );
    QSqlDatabase db = QSqlDatabase::database();

    // announce
    Common::print( CLMsg + QObject::tr( "loading database '%1'\n" ).arg( Variable::instance()->string( "databasePath" )), Common::DatabaseDebug );

    // failsafe
    if ( !db.isDriverAvailable( "QSQLITE" )) {
        Common::error( CLFatalError, QObject::tr( "sqlite not present on the system\n" ));
        return false;
    }

    // set sqlite driver
    db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setHostName( "localhost" );
    db.setDatabaseName( Main::instance()->path );

    // touch file if empty
    if ( !database.exists()) {
        database.open( QFile::WriteOnly );
        database.close();
        Common::print( CLMsg + QObject::tr( "creating non-existant database - \"%1\"\n" ).arg( dbInfo.fileName()));
    }

    // set path and open
    if ( !db.open()) {
        Common::error( CLFatalError, QObject::tr( "could not load database - \"%1\"\n" ).arg( dbInfo.fileName()));
        return false;
    }

    // create database
    if ( !Database::createStructure()) {
        Common::error( CLFatalError, QObject::tr( "could not create internal database structure\n" ));
        return false;
    }

    // delete orphaned logs on init
    Database::removeOrphanedLogs();

    // enable WAL
    QSqlQuery query;
    if ( !query.exec( "PRAGMA journal_mode=WAL" )) {
        Common::error( CLSoftError, QObject::tr( "could not enable WAL, reason: %1\n" ).arg( query.lastError().text()));
        return false;
    }

    // load entries
    if ( !Event::loadEvents())
        return false;

    Task::loadTasks();
    Team::loadTeams();
    Log::loadLogs();
    Event::buildTTList();

    // success
    return true;
}

/**
 * @brief Database::reindexTasks reindexes tasks (used when their display order is changed)
 */
void Database::reindexTasks() {
    bool reindex = false;

    // here we perform scheduled writes to disk
    foreach ( Task *task, Main::instance()->taskList ) {
        if ( task->reindexRequired()) {
            task->setOrder( task->order(), true );
            reindex = true;
        }
    }

    // announce reindexing
    if ( reindex )
        Common::print( CLMsg + QObject::tr( "performed task reindexing\n" ), Common::DatabaseDebug );
}

/**
 * @brief Database::unload safely unloads database
 */
void Database::unload() {
    QString connectionName;
    bool open = false;

    // announce
    Common::print( CLMsg + QObject::tr( "unloading database\n" ), Common::DatabaseDebug );

    // close database if open and delete orphaned logs on shutdown
    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase db = QSqlDatabase::database();
        if ( db.isOpen()) {
            open = true;
            Database::removeOrphanedLogs();
            connectionName = db.connectionName();;
            db.close();

        }
    }

    // only now we can remove the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}

/**
 * @brief Database::removeOrphanedLogs
 */
void Database::removeOrphanedLogs() {
    // create query
    QSqlQuery query;

    // announce
    Common::print( CLMsg + QObject::tr( "removing orphaned logs\n" ), Common::LogDebug );

    // remove orphaned logs (fixes crash with invalid teamId/taskId)
    if ( !query.exec( "delete from logs where value=0" ) || !query.exec( "delete from logs where teamId not in ( select id from teams )" ) || !query.exec( "delete from logs where taskId not in ( select id from tasks )" ))
        Common::error( CLSoftError, QObject::tr( "could not delete orphaned logs, reason: %1\n" ).arg( query.lastError().text()));
}

/**
 * @brief Database::writeBackup
 */
void Database::writeBackup() {
    QString backupPath;

    // make path if nonexistant
    QFileInfo db( Main::instance()->path );
    backupPath = db.absolutePath() + "/" + "backups/";
    QDir dir( backupPath );
    if ( !dir.exists()) {
        dir.mkpath( backupPath );
        if ( !dir.exists()) {
            Common::error( CLFatalError, QObject::tr( "could not create backup path\n" ));
            return;
        }
    }
    QFile::copy( Main::instance()->path, QString( "%1%2_%3.db" ).arg( backupPath ).arg( QFileInfo( Main::instance()->path ).fileName().remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
}
