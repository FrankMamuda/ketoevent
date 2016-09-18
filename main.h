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

#ifndef MAIN_H
#define MAIN_H

//
// includes
//
#include <QObject>
#include <QList>
#include <QSettings>
#include <QTime>
#include "teamentry.h"
#include "taskentry.h"
#include "consolevariable.h"
#include "evententry.h"
#include "settingsvariable.h"
#include "gui_console.h"

// message macro
#ifdef Q_CC_MSVC
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __FUNCTION__ )
#else
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __func__ )
#endif

//
// defines
//
#define FORCE_LATVIAN
#define SQL_PREPARE_STATEMENTS

// error message macros
#define StrMsg ClassFunc
#define StrFatalError Main::FatalError, ClassFunc
#define StrSoftError Main::SoftError, ClassFunc
#define StrWarn QObject::trUtf8( "WARNING:" ) + ClassFunc

// quickly print to DEBUG
#define QMD( msg ) m.print( StrMsg + QString( "%1\n" ).arg( msg ), Main::Debug );
#define QMD1( msg, arg1 ) m.print( StrMsg + QString( "%1 %2\n" ).arg( msg ).arg( arg1 ), Main::Debug );
#define QMD2( msg, arg1, arg2 ) m.print( StrMsg + QString( "%1 %2 %3\n" ).arg( msg ).arg( arg1 ).arg( arg2 ), Main::Debug );
#define QMD3( msg, arg1, arg2, arg3 ) m.print( StrMsg + QString( "%1 %2 %3 %4\n" ).arg( msg ).arg( arg1 ).arg( arg2 ).arg( arg3 ), Main::Debug );

//
// namespace: Common
//
namespace Common {
const static unsigned int API = 9;
const static unsigned int MinimumAPI = 9;
const static unsigned int defaultMinMembers = 1;
const static unsigned int defaultMaxMembers = 3;
const static unsigned int defaultComboOfTwo = 1;
const static unsigned int defaultComboOfThree = 3;
const static unsigned int defaultComboOfFourAndMore = 5;
const static unsigned int defaultPenaltyPoints = 5;
const static QString defaultStartTime( "10:00" );
const static QString defaultFinishTime( "15:00" );
const static QString defaultFinalTime( "15:30" );
const static QString defaultDatabase( "ketoevent.db" );
}

typedef struct tableField_s {
    const char *name;
    const char *type;
} tableField_t;

typedef struct table_s {
    const char *name;
    const tableField_t *fields;
    const unsigned int numFields;
} table_t;

//
// API9
//
namespace API {
// tasks
const static tableField_t taskFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 128 )" },
    { "points", "integer" },
    { "multi", "integer" },
    { "style", "integer" },
    { "type", "integer" },
    { "parent", "integer" },
    { "eventId", "integer" },
    { "description", "varchar( 512 )" }
};

// teams
const static tableField_t teamFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 64 )" },
    { "members", "integer" },
    { "finishTime", "varchar( 5 )" },
    { "lock", "integer" },
    { "reviewer", "varchar( 64 )" },
    { "eventId", "integer" }
};

// logs
const static tableField_t logFields[] = {
    { "id", "integer primary key" },
    { "value", "integer" },
    { "taskId", "integer" },
    { "teamId", "integer" },
    { "comboId", "integer" }
};

// events
const static tableField_t eventFields[] = {
    { "id", " integer primary key" },
    { "api", " integer" },
    { "name", "varchar( 64 )" },
    { "minMembers", "integer" },
    { "maxMembers", "integer" },
    { "startTime", "varchar( 5 )" },
    { "finishTime", "varchar( 5 )" },
    { "finalTime", "varchar( 5 )" },
    { "penalty", "integer" },
    { "comboOfTwo", "integer" },
    { "comboOfThree", "integer" },
    { "comboOfFourAndMore", "integer" },
    { "lock", "integer" }
};

// tables
const static table_t tables[] = {
    { "teams",  teamFields, sizeof( teamFields ) / sizeof( tableField_t ) },
    { "tasks",  taskFields, sizeof( taskFields ) / sizeof( tableField_t ) },
    { "logs",   logFields, sizeof( logFields ) / sizeof( tableField_t ) },
    { "events", eventFields, sizeof( eventFields ) / sizeof( tableField_t ) }
};
const unsigned int numTables = sizeof( tables ) / sizeof( table_t );
}

//
// API0 (2013 event)
//
namespace API0 {
// tasks
const static tableField_t taskFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 128 )" },
    { "points", "integer" },
    { "multi", "integer" },
    { "style", "integer" },
    { "type", "integer" },
    { "parent", "integer" },
};

// teams
const static tableField_t teamFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 64 )" },
    { "members", "integer" },
    { "finish", "varchar( 5 )" },
};

// logs
const static tableField_t logFields[] = {
    { "id", "integer primary key" },
    { "value", "integer" },
    { "combo", "integer" },
    { "taskId", "integer" },
    { "teamId", "integer" }
};

// tables
const static table_t tables[] = {
    { "teams",  teamFields, sizeof( teamFields ) / sizeof( tableField_t ) },
    { "tasks",  taskFields, sizeof( taskFields ) / sizeof( tableField_t ) },
    { "logs",   logFields, sizeof( logFields ) / sizeof( tableField_t ) },
};
const unsigned int numTables = sizeof( tables ) / sizeof( table_t );
}

//
// classes
//
#ifdef APPLET_DEBUG
class Gui_Console;
#endif

/**
 * @brief The Main class
 */
class Main : public QObject {
    Q_OBJECT
    Q_ENUMS( ListTypes )
    Q_ENUMS( ErrorTypes )
    Q_PROPERTY( bool initialised READ isInitialised WRITE setInitialised )
    Q_PROPERTY( DebugLevels debugLevel READ debugLevel WRITE setDebugLevel )
    Q_CLASSINFO( "description", "Applet main class" )
    Q_ENUMS( IdTypes )
    Q_ENUMS( ErrorTypes )
    Q_FLAGS( DebugLevels )

public:
    // sorting types
    enum ListTypes {
        NoType = -1,
        Teams,
        Tasks
    };

    // id types
    enum IdTypes {
        NoId = -1,
        TeamId,
        LogId,
        ComboId,
        TaskId
    };

    // error types
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

    // debug levels
    enum DebugLevel {
        NoDebug =    0x0,
        System =     0x0001,
        GuiMain =    0x0002,
        Database =   0x0004,
        LogDebug =   0x0008,
        TaskDebug =  0x0010,
        TeamDebug =  0x0020,
        EventDebug = 0x0040,
        Debug =      0x0080
    };

    enum Import {
        LogImport = 0,
        TaskImport
    };

    Q_DECLARE_FLAGS( DebugLevels, DebugLevel )

    // database related
    Log *addLog( int taskId, int teamId, int value = 0, int comboId = -1 );
    Log *logForId( int id );
    Log *logForIds( int teamId, int taskId );
    Task *taskForId( int id );
    Task *taskForName( const QString &name );
    Team *teamForId( int id, bool import = false );
    Team *teamForName( const QString &name );
    QString path;
    int highestId( IdTypes type ) const;

    // console/settings variables
    ConsoleVariable *cvar( const QString &key );
    SettingsVariable *svar( const QString &key );
    ConsoleVariable *defaultCvar;
    SettingsVariable *defaultSvar;

    // event
    Event *currentEvent();
    Event *eventForId( int id );

    // lists
    typedef struct ketoList_s {
        QList <Team*>  teamList;
        QList <Task*>  taskList;
        QList <Log*>   logList;
        QList <Event*> eventList;
    } ketoList_t;
    ketoList_t base;
    ketoList_t import;
    QList <Task*>  taskListSorted();
    QList <ConsoleVariable*>  cvarList;
    QList <SettingsVariable*> svarList;

    // misc
    QString transliterate( const QString &path );
    bool isInitialised() const { return this->m_init; }
    DebugLevels debugLevel() const { return this->m_debug; }

    // mem debug
#ifdef APPLET_DEBUG
    int alloc;
    int dealloc;
#endif

    // hashing
    static QString taskListHash( bool import );
    static QString stringToHash( const QString &input );

public slots:
    // init/shutdown
    bool initialise( QObject *parent );
    void setInitialised( bool init = true ) { this->m_init = init; }
    void shutdown( bool ignoreDatabase = false );

    // database
    bool createDatabaseStructure( const QString &prefix = "" );
    void unloadDatabase();

    // debug
    void setDebugLevel( DebugLevels debug ) { this->m_debug = debug; }

    // database related
    void addTeam( const QString &teamName, int members, QTime finishTime, const QString &reviewerName = QString::null, bool lockState = false );
    void addTask( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style = Task::NoStyle , const QString &description = QString::null );
    void addEvent( const QString &title = QString::null );
    void attachDatabase( const QString &path, Import = LogImport );
    void removeTeam( const QString &teamName );
    void reindexTasks();

    // console io
    void error( ErrorTypes type = SoftError, const QString &func = "", const QString &msg = "" );
    void print( const QString &msg, DebugLevel = Main::NoDebug );

    // misc
    void sort( ListTypes type );
    void update();
#ifdef APPLET_DEBUG
    void initConsole();
#endif
    void clearEvent();
    void reloadDatabase( const QString &path ) { this->unloadDatabase(); this->makePath( path ); this->loadDatabase(); }
    bool setCurrentEvent( Event *eventPtr );
    void buildEventTTList();
#ifdef APPLET_DEBUG
    void showConsole() { this->console->show(); }
    void hideConsole() { this->console->hide(); }
#endif

    // combos
    int getFreeComboHandle() const;

private:
    QSettings *settings;
    int changesCounter;
    Event *m_event;
    bool m_init;
#ifdef APPLET_DEBUG
    Gui_Console *console;
#endif
    DebugLevels m_debug;

private slots:
    // console/settings variables
    void addCvar( ConsoleVariable *varPtr );
    void addSvar( const QString &key, SettingsVariable::Types type, SettingsVariable::Class varClass );

    // database related
    bool loadDatabase();
    void makePath( const QString &path = QString::null );
    void loadTasks( bool import = false, bool store = false );
    void loadTeams( bool import = false, bool store = false );
    void loadLogs( bool import = false, bool store = false );
    bool loadEvents( bool import = false );
    void removeOrphanedLogs();
    void writeBackup();
    bool createEmptyTable( const QString &prefix = QString::null );
};

// flags
Q_DECLARE_OPERATORS_FOR_FLAGS( Main::DebugLevels )

//
// externals
//
extern class Main m;

#endif // MAIN_H
