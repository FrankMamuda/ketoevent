/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
// database.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QCryptographicHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QMessageBox>

/**
 * @brief Main::makePath makes a filesystem path if non existant
 * @param path filesystem path
 */
void Main::makePath( const QString &path ) {
    QString fullPath;

    // default path?
    if ( path.isEmpty()) {
#ifdef Q_OS_UNIX
        fullPath = QString( QDir::homePath() + "/.ketoevent/" );
#else
        fullPath = QString( QDir::currentPath() + "/" );
#endif
        fullPath.append( "/" );
        fullPath.append( Common::defaultDatabase );
    } else
        fullPath = path;

    // make path id nonexistant
    QFileInfo db( fullPath );
    QDir dir;
    dir.setPath( db.absolutePath());

    if ( !dir.exists()) {
        dir.mkpath( db.absolutePath());
        if ( !dir.exists())
            this->error( StrFatalError, this->tr( "could not create database path - \"%1\"\n" ).arg( fullPath ));
    }

    // store path
    this->path = fullPath;
    this->path.replace( "//", "/" );
}

/**
 * @brief Main::highestId returns highest id in database either for combo, task, etc.
 * @param type entry type
 * @return id
 */
int Main::highestId( IdTypes type ) const {
    int id = 1;

    switch ( type ) {
    case ComboId:
        foreach ( Log *logPtr, m.base.logList ) {
            if ( logPtr->comboId() > id )
                id = logPtr->id();
        }
        break;

    case TeamId:
        foreach ( Team *teamPtr, m.base.teamList ) {
            if ( teamPtr->id() > id )
                id = teamPtr->id();
        }
        break;

    case LogId:
        foreach ( Log *logPtr, m.base.logList ) {
            if ( logPtr->id() > id )
                id = logPtr->id();
        }
        break;

    case TaskId:
        foreach ( Task *taskPtr, m.base.taskList ) {
            if ( taskPtr->id() > id )
                id = taskPtr->id();
        }
        break;


    case NoId:
    default:
        break;
    }
    return id;
}

/**
 * @brief Main::stringToHash returns md5 of a string
 * @param input text string
 * @return md5 of the given string
 */
QString Main::stringToHash( const QString &input ) {
    QCryptographicHash *hash = new QCryptographicHash( QCryptographicHash::Md5 );
    hash->addData( input.toLatin1().constData(), input.length());
    return QString( hash->result().toHex().constData());
}

/**
 * @brief taskListHash builds a unique task list md5 string to validate imports
 * @param import database input toggle
 * @return task list md5 hash
 */
QString Main::taskListHash( bool import ) {
    QList<Task*> list;
    QString taskString;
    int eventId = m.currentEvent()->id();

    if ( import )
        list = m.import.taskList;
    else
        list = m.currentEvent()->taskList;

    // build unique taskList
    foreach ( Task *taskPtr, list ) {
        if ( taskPtr->eventId() != eventId )
            continue;

        taskString.append( QString( "%1%2%3%4" ).arg( taskPtr->name().remove( " (imported)")).arg( taskPtr->points()).arg( taskPtr->multi()).arg( taskPtr->type()));
    }

    // generate taskList checksum (to avoid mismatches)
    return Main::stringToHash( taskString );
}

/**
 * @brief Main::attachDatabase attaches a database to be imported
 * @param path database path in filesystem
 * @param import import options (logs, teams, etc.)
 */
void Main::attachDatabase( const QString &path, Import import ) {
    QSqlQuery query;
    QString dbPath = path + "import";
    bool store = true;
    int eventId = -1;

    // announce
    m.print( StrMsg + this->tr( "attaching database %1\n" ).arg( path ), Main::Database );

    // write backup just in case
    this->writeBackup();

    // check database
    QFile::copy( path, dbPath );
    QFile database( dbPath );
    QFileInfo dbInfo( database );
    if ( !database.exists()) {
        this->error( StrSoftError, this->tr( "database \"%1\" does not exist\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // attach the new database
    if ( !query.exec( QString( "attach '%1' as merge" ).arg( dbPath ))) {
        this->error( StrSoftError, this->tr( "could not attach database, reason - \"%1\"\n" ).arg( query.lastError().text()));
        goto removeDB;
    }

    if ( import == LogImport ) {
        // update teams
        if ( !query.exec( QString( "update merge.teams set id=id*-1" )) || !query.exec( QString( "update merge.teams set id=(id*-1)+%1" ).arg( this->highestId( TeamId )))) {
            this->error( StrSoftError, this->tr( "could not update teams, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }

        // update logs
        if ( !query.exec( QString( "update merge.logs set id=id*-1" )) || !query.exec( QString( "update merge.logs set id=(id*-1)+%1" ).arg( this->highestId( LogId )))) {
            this->error( StrSoftError, this->tr( "could not update logs, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }
    } else if ( import == TaskImport ) {
        store = false;

        // update tasks
        if ( !query.exec( QString( "update merge.tasks set id=id*-1" )) || !query.exec( QString( "update merge.tasks set id=(id*-1)+%1" ).arg( this->highestId( TaskId )))) {
            this->error( StrSoftError, this->tr( "could not update tasks, reason - \"%1\"\n" ).arg( query.lastError().text()));
            goto removeDB;
        }
    }

    // load eventList into temporary storage
    if ( !this->loadEvents( true )) {
        this->error( StrSoftError, this->tr( "could not load database \"%1\"\n" ).arg( dbInfo.fileName()));
        goto removeDB;
    }

    // find event by name
    foreach ( Event *eventPtr, m.import.eventList ) {
        if ( !QString::compare( eventPtr->name(), m.currentEvent()->name())) {
            eventId = eventPtr->id();
            break;
        }
    }

    // failsafe
    if ( eventId == -1 ) {
        this->error( StrSoftError, this->tr( "database \"%1\" does not contain event \"%2\"\n" ).arg( dbInfo.fileName()).arg( m.currentEvent()->name()));
        goto removeDB;
    }

    // get rid of junk
    query.exec( QString( "delete from merge.events where id!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.teams where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.logs where eventId!=%1" ).arg( eventId ));
    query.exec( QString( "delete from merge.tasks where eventId!=%1" ).arg( eventId ));

    // update connections
    query.exec( QString( "update merge.teams set eventId=%1" ).arg( m.currentEvent()->id()));
    query.exec( QString( "update merge.tasks set eventId=%1" ).arg( m.currentEvent()->id()));
    query.exec( QString( "update merge.logs set teamId=teamId+%1" ).arg( this->highestId( TeamId )));
    query.exec( QString( "update merge.logs set comboId=comboId+%1 where comboId!=-1" ).arg( this->highestId( LogId )));

    // load taskList into temporary storage
    this->loadTasks( true, !store );

    // compare task hashes
    if ( import == LogImport ) {
        QString one = Main::taskListHash( true );
        QString two = Main::taskListHash( false );

        if ( QString::compare( one, two )) {
            this->error( StrSoftError, this->tr( "task list mismatch \"%1\", \"%2\"\n" )
                         .arg( one ).arg( two ));
            goto removeDB;
        }

        // load teamlist into temporary storage
        this->loadTeams( true, store );

        // load logs into temporary storage
        this->loadLogs( true, store );
    }

    // clean up
    this->import.teamList.clear();
    this->import.logList.clear();
    this->import.taskList.clear();
    this->import.eventList.clear();

removeDB:
    query.exec( "detach merge" );
    database.remove();
}

/**
 * @brief Main::createDatabaseStructure creates an empty, structured database
 * @param prefix import prefix
 */
bool Main::createDatabaseStructure( const QString &prefix ) {
    unsigned int y, k, l, n;
    QSqlQuery query;

    // failafe
    QFile dbFile( this->cvar( "databasePath" )->string());
    if ( !dbFile.exists()) {
        this->error( StrFatalError, this->tr( "unable to create database file\n" ));
        return false;
    }

    // additional structure check (API9)
    QSqlDatabase db = QSqlDatabase::database();
    QStringList tables = db.tables();
    bool mismatch = false;

    // announce
    if ( !tables.count())
        m.print( StrMsg + this->tr( "creating an empty database '%1'\n" ).arg( this->cvar( "databasePath" )->string()), Main::Database );

    // TODO: api mismatch should also be handled here (reading a single field from events table)
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
        m.print( StrMsg + this->tr( "Table field mismatch in '%1'. Different API?\n" ).arg( this->cvar( "databasePath" )->string()), Main::Database );

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

            msgBox.setText( this->tr( "Attempt to import API0 (2013) database?" ));
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msgBox.setDefaultButton( QMessageBox::Yes );
            msgBox.setIcon( QMessageBox::Warning );
            state = msgBox.exec();

            // check options
            switch ( state ) {
            case QMessageBox::Yes:
                // create initial table structure (temporary)
                if ( !this->createEmptyTable( "tmp_" ))
                    return false;

                // go through all tables and all fields
                for ( y = 0; y < API::numTables; y++ ) {
                    table_t api9 = API::tables[y];

                    for ( k = 0; k < API0::numTables; k++ ) {
                        table_t api0 = API0::tables[k];

                        if ( !QString::compare( api9.name, api0.name, Qt::CaseInsensitive )) {
                            QStringList fields;

                            for ( l = 0; l < api9.numFields; l++ ) {
                                tableField_t api9f = api9.fields[l];

                                for ( n = 0; n < api0.numFields; n++ ) {
                                    tableField_t api0f = api0.fields[n];

                                    // generate a list of matching fields
                                    if ( !QString::compare( api9f.name, api0f.name, Qt::CaseInsensitive ))
                                        fields << api9f.name;
                                }
                            }

                            // copy data (tested only for API0)
                            if ( !QString::compare( api9.name, "teams", Qt::CaseInsensitive ))
                                query.exec( QString( "insert into tmp_%1 ( %2, finishTime ) select %2, finish from %1" ).arg( api9.name ).arg( fields.join( ", " )));
                            else
                                query.exec( QString( "insert into tmp_%1 ( %2 ) select %2 from %1" ).arg( api9.name ).arg( fields.join( ", " )));
                        }
                    }

                    // drop legacy tables, rename temporary tables to permanent
                    query.exec( QString( "drop table %1" ).arg( api9.name ));
                    query.exec( QString( "alter table tmp_%1 rename to %1" ).arg( api9.name ));
                }

                // add a single event
                m.addEvent( "Imported 2013 event" );

                // fill NULL values with blank data
                // TODO: also load combos as a sum of all combopoints into a new task "IMPORTED COMBOS" (multi)
                query.exec( "update teams set lock=0" );
                query.exec( "update teams set reviewer='imported'" );
                query.exec( "update teams set eventId=1" );
                query.exec( "update tasks set eventId=1" );
                query.exec( "update tasks set description=''" );
                query.exec( "update logs set comboId=-1" );
                return true;
                break;

            case QMessageBox::No:
            default:
                ;
            }
        } else
            this->error( StrFatalError, this->tr( "Could not import database\n" ));

        return false;
    }

    // create initial table structure (if non-existant)
    return this->createEmptyTable( prefix );
}

/**
 * @brief Main::createEmptyTable
 * @param prefix
 * @param suffix
 * @return
 */
bool Main::createEmptyTable( const QString &prefix ) {
    QSqlQuery query;

    // TODO: check if schema is matching
    // TODO: generate schema from API tables
    if ( !query.exec( QString( "create table if not exists %1tasks ( id integer primary key, name varchar( 128 ), points integer, multi integer, style integer, type integer, parent integer, eventId integer, description varchar( 512 ))" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1teams ( id integer primary key, name varchar( 64 ), members integer, finishTime varchar( 5 ), lock integer, reviewer varchar( 64 ), eventId integer )" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1events ( id integer primary key, api integer, name varchar( 64 ), minMembers integer, maxMembers integer, startTime varchar( 5 ), finishTime varchar( 5 ), finalTime varchar( 5 ), penalty integer, comboOfTwo integer, comboOfThree integer, comboOfFourAndMore integer, lock integer )" ).arg( prefix )) ||
         !query.exec( QString( "create table if not exists %1logs ( id integer primary key, value integer, taskId integer, teamId integer, comboId integer )" ).arg( prefix ))) {
        this->error( StrFatalError, this->tr( "could not create internal database structure, reason - \"%1\"\n" ).arg( query.lastError().text()));
        return false;
    }
    return true;
}

/**
 * @brief Main::loadDatabase reads sqlite database from filesystem
 */
bool Main::loadDatabase() {
    // create database
    QFile database( this->path );
    QFileInfo dbInfo( database );
    QSqlDatabase db = QSqlDatabase::database();

    // announce
    m.print( StrMsg + this->tr( "loading database '%1'\n" ).arg( this->cvar( "databasePath" )->string()), Main::Database );

    // failsafe
    if ( !db.isDriverAvailable( "QSQLITE" )) {
        this->error( StrFatalError, this->tr( "sqlite not present on the system\n" ));
        return false;
    }

    // set sqlite driver
    db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setHostName( "localhost" );
    db.setDatabaseName( this->path );

    // touch file if empty
    if ( !database.exists()) {
        database.open( QFile::WriteOnly );
        database.close();
        this->print( StrMsg + this->tr( "creating non-existant database - \"%1\"\n" ).arg( dbInfo.fileName()));
    }

    // set path and open
    if ( !db.open()) {
        this->error( StrFatalError, this->tr( "could not load database - \"%1\"\n" ).arg( dbInfo.fileName()));
        return false;
    }

    // create database
    if ( !this->createDatabaseStructure()) {
        this->error( StrFatalError, this->tr( "could not create internal database structure\n" ));
        return false;
    }

    // delete orphaned logs on init
    this->removeOrphanedLogs();

    // enable WAL
    QSqlQuery query;
    if ( !query.exec( "PRAGMA journal_mode=WAL" )) {
        this->error( StrSoftError, QString( "could not enable WAL, reason: %1\n" ).arg( query.lastError().text()));
        return false;
    }

    // load entries
    if ( !this->loadEvents())
        return false;

    this->loadTasks();
    this->loadTeams();
    this->loadLogs();
    this->buildEventTTList();

    // success
    return true;
}

/**
 * @brief Main::reindexTasks reindexes tasks (used when their display order is changed)
 */
void Main::reindexTasks() {
    bool reindex = false;

    // here we perform scheduled writes to disk
    foreach ( Task *taskPtr, m.base.taskList ) {
        if ( taskPtr->reindexRequired()) {
            taskPtr->setOrder( taskPtr->order(), true );
            reindex = true;
        }
    }

    // announce reindexing
    if ( reindex )
        m.print( StrMsg + this->tr( "performed task reindexing\n" ), Main::Database );
}

/**
 * @brief Main::unloadDatabase safely unloads database
 */
void Main::unloadDatabase() {
    QString connectionName;
    bool open = false;

    // announce
    m.print( StrMsg + this->tr( "unloading database\n" ), Main::Database );

    // close database if open and delete orphaned logs on shutdown
    // according to Qt5 documentation, this must be out of scope
    {
        QSqlDatabase db = QSqlDatabase::database();
        if ( db.isOpen()) {
            open = true;
            this->removeOrphanedLogs();
            connectionName = db.connectionName();;
            db.close();

        }
    }

    // only now we can remove the connection completely
    if ( open )
        QSqlDatabase::removeDatabase( connectionName );
}
