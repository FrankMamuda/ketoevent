/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "main.h"
#include "gui_main.h"
#include <QApplication>
#include <QDir>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include <QTranslator>

//
// classes
//
class Main m;

//
// defines
//
#define FORCE_LATVIAN

/*
================
initialize
================
*/
void Main::initialize() {
    // init counters
    this->changesCounter = 0;

    // initialize settings
    this->settings = new QSettings( "avoti", "ketoevent3" );
    this->settings->setDefaultFormat( QSettings::NativeFormat );

    // init cvars
    this->addVariable( new ConsoleVariable( "members/min", this->settings, 2 ));
    this->addVariable( new ConsoleVariable( "members/max", this->settings, 3 ));
    this->addVariable( new ConsoleVariable( "time/start", this->settings, QTime( 10, 0 )));
    this->addVariable( new ConsoleVariable( "time/finish", this->settings, QTime( 15, 00 )));
    this->addVariable( new ConsoleVariable( "time/final", this->settings, QTime( 15, 30 )));
    this->addVariable( new ConsoleVariable( "combo/single", this->settings, 1 ));
    this->addVariable( new ConsoleVariable( "combo/double", this->settings, 3 ));
    this->addVariable( new ConsoleVariable( "combo/triple", this->settings, 5 ));
    this->addVariable( new ConsoleVariable( "penaltyMultiplier", this->settings, 5 ));
    this->addVariable( new ConsoleVariable( "backup/perform", this->settings, true ));
    this->addVariable( new ConsoleVariable( "backup/changes", this->settings, 25 ));

#if 0
    this->addVariable( new ConsoleVariable( "misc/sortTasks", this->settings, false ));
#endif

    // load database entries
    this->loadDatabase();
}

/*
================
var
================
*/
ConsoleVariable *Main::var( const QString &key ) {
    foreach ( ConsoleVariable *varPtr, this->varList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return NULL;
}

/*
================
addVariable
================
*/
void Main::addVariable( ConsoleVariable *varPtr ) {
    // avoid duplicates
    if ( this->var( varPtr->key())) {
        delete varPtr;
        return;
    }

    this->varList << varPtr;
}

/*
============
print
============
*/
void Main::print( const QString &msg ) {
    // for debugging
    if ( msg.endsWith( "\n" ))
        qDebug() << msg.left( msg.length()-1 );
    else
        qDebug() << msg;
}

/*
============
error
============
*/
void Main::error( ErrorTypes type, const QString &msg ) {
    if ( type == FatalError ) {
        this->print( this->tr( "FATAL ERROR: %1" ).arg( msg ));
        this->shutdown( true );
    } else
        this->print( this->tr( "ERROR: %1" ).arg( msg ));
}

/*
================
shutdown
================
*/
void Main::shutdown( bool ignoreDatabase ) {
    QSqlQuery query;

    // save settings
    this->settings->sync();
    delete this->settings;

    // delete orphaned logs on shutdown
    if ( !ignoreDatabase ) {
        this->deleteOrphanedLogs();

        // close database
        QSqlDatabase db = QSqlDatabase::database();
        db.close();
    }

    // clear console vars
    foreach ( ConsoleVariable *varPtr, this->varList )
        delete varPtr;
    this->varList.clear();

    // close applet
    QApplication::quit();
}

/*
================
addTeam
================
*/
void Main::addTeam( const QString &teamName, int members, QTime finishTime ) {
    QSqlQuery query;

    // avoid duplicates
    if ( m.teamForName( teamName ) != NULL )
        return;

    // perform database update and select last row
    if ( !query.exec( QString( "insert into teams values ( null, '%1', %2, '%3' )" )
                      .arg( teamName )
                      .arg( members )
                      .arg( finishTime.toString( "hh:mm" ))
                      )) {
        this->error( StrSoftError + QString( "could not add team, reason: \"%1\"\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from teams where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last entry and construct internal entry
    while ( query.next())
        this->teamList << new TeamEntry( query.record(), "teams" );
}

/*
================
removeTeam
================
*/
void Main::removeTeam( const QString &teamName ) {
    TeamEntry *teamPtr = NULL;
    QSqlQuery query;

    // find team
    teamPtr = m.teamForName( teamName );

    // failsafe
    if ( teamPtr == NULL )
        return;

    // remove team and logs from db
    query.exec( QString( "delete from teams where id=%1" ).arg( teamPtr->id()));
    query.exec( QString( "delete from logs where teamId=%1" ).arg( teamPtr->id()));

    // remove from display
    this->teamList.removeAll( teamPtr );
}

/*
================
addTask
================
*/
void Main::addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style ) {
    QSqlQuery query;
    int max = 0;

    // avoid duplicates
    if ( m.taskForName( taskName ) != NULL )
        return;

    // make sure we insert value at the bottom of the list
    query.exec( "select max ( parent ) from tasks" );
    while ( query.next())
        max = query.value( 0 ).toInt();

    // perform database update and select last row
    if ( !query.exec( QString( "insert into tasks values ( null, '%1', %2, %3, %4, %5, %6 )" )
                      .arg( taskName )
                      .arg( points )
                      .arg( multi )
                      .arg( static_cast<TaskEntry::Styles>( style ))
                      .arg( static_cast<TaskEntry::Types>( type ))
                      .arg( max + 1 )
                      )) {
        this->error( StrSoftError + QString( "could not add task, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from tasks where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next())
        this->taskList << new TaskEntry( query.record(), "tasks" );
}

/*
================
addLog
================
*/
LogEntry *Main::addLog( int taskId, int teamId, int value, LogEntry::Combos combo ) {
    LogEntry *logPtr = NULL;
    QSqlQuery query;

    // avoid duplicates
    foreach ( logPtr, this->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }

    // add new log
    if ( !query.exec( QString( "insert into logs values ( null, %1, %2, %3, %4 )" )
                      .arg( value )
                      .arg( static_cast<int>( combo ))
                      .arg( taskId )
                      .arg( teamId )
                      )) {
        this->error( StrSoftError + QString( "could not add log, reason: %1\n" ).arg( query.lastError().text()));
    }
    query.exec( QString( "select * from logs where id=%1" ).arg( query.lastInsertId().toInt() ));

    // get last entry and construct internal entry
    while ( query.next()) {
        logPtr = new LogEntry( query.record(), "logs" );
        this->logList << logPtr;
    }
    return logPtr;
}

/*
================
loadDatabase
================
*/
void Main::loadDatabase() {
#ifdef Q_OS_UNIX
#ifdef Q_OS_ANDROID
    this->path = QString( "/sdcard/data/org.factory12.ketoevent3/" );
#else
    this->path = QString( QDir::homePath() + "/.ketoevent/" );
#endif
#else
    this->path = QString( QDir::currentPath() + "/" );
#endif

    // make path id nonexistant
    QDir dir( this->path );
    if ( !dir.exists()) {
        dir.mkpath( this->path );
        if ( !dir.exists()) {
            m.error( StrFatalError + this->tr( "could not create database path\n" ));
            return;
        }
    }

    // create database

    this->databasePath = this->path + "ketoevent.db";
    QFile database( this->databasePath );
    QSqlDatabase db;

    // failsafe
    if ( !db.isDriverAvailable( "QSQLITE" ))
        m.error( StrFatalError + this->tr( "sqlite not present on the system\n" ));

    // set sqlite driver
    db = QSqlDatabase::addDatabase( "QSQLITE" );

    // touch file if empty
    if ( !database.exists()) {
        database.open( QFile::WriteOnly );
        database.close();
    }

    // set path and open
    db.setDatabaseName( this->databasePath );
    if ( !db.open())
        m.error( StrFatalError + this->tr( "could not load task database\n" ));

    // create query
    QSqlQuery query;

    // create initial table structure (if non-existant)
    if ( !query.exec( "create table if not exists tasks ( id integer primary key, name varchar( 256 ) unique, points integer, multi integer, style integer, type integer, parent integer )" ) ||
         !query.exec( "create table if not exists teams ( id integer primary key, name varchar( 64 ) unique, members integer, finish varchar( 5 ))" ) ||
         !query.exec( "create table if not exists logs ( id integer primary key, value integer, combo integer, taskId integer, teamId integer )" )
         ) {
        m.error( StrFatalError + this->tr( "could not create internal database structure\n" ));
    }

    // delete orphaned logs on init
    this->deleteOrphanedLogs();

    // load entries
    this->loadTasks();
    this->loadTeams();
    this->loadLogs();
}

/*
================
deleteOrphanedLogs
================
*/
void Main::deleteOrphanedLogs() {
    // create query
    QSqlQuery query;

    // remove orphaned logs (fixes crash with invalid teamId/taskId)
    if ( !query.exec( "delete from logs where value=0" ) || !query.exec( "delete from logs where teamId not in ( select id from teams )" ) || !query.exec( "delete from logs where taskId not in ( select id from tasks )" ))
        m.error( StrSoftError + QString( "could not delete orphaned logs, reason: %1\n" ).arg( query.lastError().text()));
}

/*
================
importDatabase (testing)
================
*/
void Main::importDatabase( const QString &path ) {
    QList<QPair<int, QString> > teamMatchList;
    QList<QPair<int, QString> > taskMatchList;

    // create query
    QSqlQuery query;

    // attach the new database
    query.exec( QString( "attach '%1' as toMerge" ).arg( path ));

    //
    // first add teams
    //
    query.exec( "select * from toMerge.teams" );

    // store entries
    while ( query.next()) {
        QString teamName = query.record().value( "name" ).toString();

        // check for duplicates
        if ( m.teamForName( teamName ) != NULL ) {
            // TODO: messagebox - replace?
            QMessageBox::question( NULL, "Replace team", QString( "Replace logs for team \"%1\"?" ).arg( teamName ), QMessageBox::Yes, QMessageBox::No );
            this->removeTeam( teamName );
        }

        // store temp value
        QList<QPair<int, QString> > teamMatch;
        teamMatch.append( qMakePair( query.record().value( "id" ).toInt(), teamName ));
        teamMatchList << teamMatch;

        // add to database
        this->addTeam( query.record().value( "name" ).toString(), query.record().value( "members" ).toInt(), QTime::fromString( query.record().value( "finish" ).toString(), "hh:mm" ));
    }

    //
    // then add tasks
    //
    query.exec( "select * from toMerge.tasks" );

    // store entries
    while ( query.next()) {
        // store temp value
        QList<QPair<int, QString> > taskMatch;
        taskMatch.append( qMakePair( query.record().value( "id" ).toInt(), query.record().value( "name" ).toString()));
        taskMatchList << taskMatch;

        // add to main database
        // duplicates are ignored
        this->addTask( query.record().value( "name" ).toString(), query.record().value( "points" ).toInt(), query.record().value( "multi" ).toInt(), static_cast<TaskEntry::Types>( query.record().value( "type" ).toInt()), static_cast<TaskEntry::Styles>( query.record().value( "style" ).toInt()));
    }

    //
    // next part is tricky one:
    // we have to match log by old taskId and teamId
    // I'm sure it can be done more efficiently
    //
    QPair<int, QString> teamMatchPtr;
    foreach ( teamMatchPtr, teamMatchList ) {
        TeamEntry *teamPtr;

        // first find the new team (if none, don't bother)
        teamPtr = m.teamForName( teamMatchPtr.second );
        if ( teamPtr == NULL )
            return;

        // then get all logs for the team
        query.exec( QString( "select * from toMerge.logs where teamId=%1" ).arg( teamMatchPtr.first ));

        // cycle through logs
        while ( query.next()) {
            // first find the task (it may be imported or already existing)
            QPair<int, QString> taskMatchPtr;
            foreach ( taskMatchPtr, taskMatchList ) {
                if ( taskMatchPtr.first == query.record().value( 2 ).toInt())
                    break;
            }
            TaskEntry *taskPtr = m.taskForName( taskMatchPtr.second );
            if ( taskPtr == NULL )
                return;

            // then add log
            this->addLog( taskPtr->id(), teamPtr->id(), query.record().value( "value" ).toInt(), static_cast<LogEntry::Combos>( query.record().value( "combo" ).toInt()));
        }
    }

    // detach the new database
    query.exec( "detach toMerge" );
}

/*
================
loadTasks
================
*/
void Main::loadTasks() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from tasks order by parent asc" );

    // store entries
    while ( query.next())
        this->taskList << new TaskEntry( query.record(), "tasks" );
}

/*
================
loadTeams
================
*/
void Main::loadTeams() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from teams" );

    // store entries
    while ( query.next())
        this->teamList << new TeamEntry( query.record(), "teams" );

    // sort alphabetically
    this->sort( Main::Teams );
}

/*
================
loadLogs
================
*/
void Main::loadLogs() {
    QSqlQuery query;

    // read stuff
    query.exec( "select * from logs" );

    // store entries
    while ( query.next()) {
        LogEntry *logPtr = new LogEntry( query.record(), "logs" );
        this->teamForId( logPtr->teamId())->logList << logPtr;
        this->logList << logPtr;
    }
}

/*
================
logForId
================
*/
LogEntry *Main::logForId( int id ) {
    foreach ( LogEntry *logPtr, this->logList ) {
        if ( logPtr->id() == id )
            return logPtr;
    }
    return NULL;
}

/*
================
logForIds
================
*/
LogEntry *Main::logForIds( int teamId, int taskId ) {
    TeamEntry *teamPtr = this->teamForId( teamId );
    if ( teamPtr == NULL )
        return NULL;

    foreach ( LogEntry *logPtr, teamPtr->logList ) {
        if ( logPtr->taskId() == taskId && logPtr->teamId() == teamId )
            return logPtr;
    }
    return NULL;
}

/*
================
taskForId
================
*/
TaskEntry *Main::taskForId( int id ) {
    foreach ( TaskEntry *taskPtr, this->taskList ) {
        if ( taskPtr->id() == id )
            return taskPtr;
    }
    return NULL;
}

/*
================
taskForName
================
*/
TaskEntry *Main::taskForName( const QString &name ) {
    foreach ( TaskEntry *taskPtr, this->taskList ) {
        if ( !QString::compare( name, taskPtr->name()))
            return taskPtr;
    }
    return NULL;
}

/*
================
teamForId
================
*/
TeamEntry *Main::teamForId( int id ) {
    foreach ( TeamEntry *teamPtr, this->teamList ) {
        if ( teamPtr->id() == id )
            return teamPtr;
    }
    return NULL;
}

/*
================
teamForName
================
*/
TeamEntry *Main::teamForName( const QString &name ) {
    foreach ( TeamEntry *teamPtr, this->teamList ) {
        if ( !QString::compare( name, teamPtr->name()))
            return teamPtr;
    }
    return NULL;
}

// latin4 chars
static unsigned int latin4Array[] = {
    224, // aa
    232, // ch
    186, // ee
    187, // gj
    239, // ii
    243, // kj
    182, // lj
    241, // nj
    242, // oo
    179, // rj
    185, // sh
    254, // uu
    190, // zh
    192, // AA
    200, // CH
    170, // EE
    171, // GJ
    207, // II
    211, // KJ
    166, // LJ
    209, // NJ
    210, // OO
    163, // RJ
    169, // SH
    222, // UU
    174  // ZH
};
static int latin4ArraySize = sizeof( latin4Array ) / sizeof( int );


// latin4 chars
static unsigned int latin4ArrayB[] = {
    257, // aa
    269, // ch
    275, // ee
    291, // gj
    299, // ii
    311, // kj
    316, // lj
    326, // nj
    333, // oo
    343, // rj
    353, // sh
    363, // uu
    382, // zh
    256, // AA
    268, // CH
    274, // EE
    290, // GJ
    298, // II
    310, // KJ
    315, // LJ
    325, // NJ
    332, // OO
    342, // RJ
    352, // SH
    362, // UU
    381  // ZH
};

// latin1 corresponding chars
static char latin1Array[] = {
    'a', // aa
    'c', // ch
    'e', // ee
    'g', // gj
    'i', // ii
    'k', // kj
    'l', // lj
    'n', // nj
    'o', // oo
    'r', // rj
    's', // sh
    'u', // uu
    'z', // zh
    'A', // AA
    'C', // CH
    'E', // EE
    'G', // GJ
    'I', // II
    'K', // KJ
    'L', // LJ
    'N', // NJ
    'O', // OO
    'R', // RJ
    'S', // SH
    'U', // UU
    'Z'  // ZH
};

/*
============
transliterate
============
*/
QString Main::transliterate( const QString &path ) {
    int y;
    QString out;

    foreach ( QChar ch, path ) {
        for ( y = 0; y < latin4ArraySize; y++ ) {
            if ( ch == QChar( latin4Array[y] ) || ch == QChar( latin4ArrayB[y] ))
                ch = latin1Array[y];
        }
        out.append( ch );
    }
    return out;
}

/*
================
listToAscending
================
*/
template <class T>
bool listToAscending( T *ePtr0, T *ePtr1 ) {
    return m.transliterate( ePtr0->name().toLower()) < m.transliterate( ePtr1->name().toLower());
}

/*
================
sort
================
*/
void Main::sort( ListTypes type ) {
    switch ( type ) {
    case Tasks:
        qSort( this->taskList.begin(), this->taskList.end(), listToAscending<TaskEntry> );
        break;

    case Teams:
        qSort( this->teamList.begin(), this->teamList.end(), listToAscending<TeamEntry> );
        break;

    case NoType:
    default:
        m.error( StrSoftError + this->tr( "unknown list type \"%1\"\n" ).arg( static_cast<int>( type )));
        return;
    }
}

/*
================
writeBackup
================
*/
void Main::writeBackup() {
    QString backupPath;

    // make path id nonexistant
    backupPath = this->path + "backups/";
    QDir dir( backupPath );
    if ( !dir.exists()) {
        dir.mkpath( backupPath );
        if ( !dir.exists()) {
            m.error( StrFatalError + this->tr( "could not create backup path\n" ));
            return;
        }
    }
    QFile::copy( this->databasePath, QString( "%1%2.db" ).arg( backupPath ).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
}

/*
================
update
================
*/
void Main::update() {
    this->changesCounter++;

    if ( this->changesCounter == this->var( "backup/changes" )->value().toInt() && this->var( "backup/perform" )->value().toBool()) {
        this->writeBackup();
        this->changesCounter = 0;
    }
}

/*
================
entry point
================
*/
int main( int argc, char *argv[] ) {
    QApplication app( argc, argv );

    // init app
    QCoreApplication::setOrganizationName( "avoti" );
    QCoreApplication::setApplicationName( "ketoevent3" );

    // i18n
    QTranslator translator;
    QString locale;
#ifndef FORCE_LATVIAN
    locale = QLocale::system().name();
#else
    locale = "lv_LV";
#endif
    translator.load( QString( ":/i18n/ketoevent_%1" ).arg( locale ));
    app.installTranslator( &translator );

    // init main window
    Gui_Main gui;
#ifdef Q_OS_ANDROID
    app.setStyleSheet( "QTimeEdit { width: 128px; } QTimeEdit::up-button { subcontrol-position: right; width: 32px; height: 32px; } QTimeEdit::down-button { subcontrol-position: left; width: 32px; height: 32px; } QSpinBox { width: 96px; } QSpinBox::up-button { subcontrol-position: right; width: 32px; height: 32px; } QSpinBox::down-button { subcontrol-position: left; width: 32px; height: 32px; } QCheckBox::indicator { width: 32px; height: 32px; }" );
    gui.showMaximized();
#else
    gui.show();
#endif

    // initialize application
    m.initialize();

    // add teams
    gui.initialize();

    // exec app
    return app.exec();
}
