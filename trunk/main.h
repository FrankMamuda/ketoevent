/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
#define APPLET_DEBUG

#ifdef APPLET_DEBUG
#include <QDebug>
#endif

// error message macros
#define StrMsg ClassFunc
#define StrFatalError Main::FatalError, ClassFunc
#define StrSoftError Main::SoftError, ClassFunc
#define StrWarn QObject::trUtf8( "WARNING:" ) + ClassFunc

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

//
// class:Main
//
class Main : public QObject {
    Q_OBJECT
    Q_ENUMS( ListTypes )
    Q_ENUMS( ErrorTypes )
    Q_PROPERTY( bool initialized READ isInitialized WRITE setInitialized )
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
        NoDebug =   0x0,
        System =    0x0001,
        GuiMain =   0x0002,
        Database =  0x0004
    };

    enum Import {
        LogImport = 0,
        TaskImport
    };

    Q_DECLARE_FLAGS( DebugLevels, DebugLevel )

    // database related
    LogEntry *addLog( int taskId, int teamId, int value = 0, int comboId = -1 );
    LogEntry *logForId( int id );
    LogEntry *logForIds( int teamId, int taskId );
    TaskEntry *taskForId( int id );
    TaskEntry *taskForName( const QString &name );
    TeamEntry *teamForId( int id, bool import = false );
    TeamEntry *teamForName( const QString &name );
    QString path;
    int highestId( IdTypes type ) const;

    // console/settings variables
    ConsoleVariable *cvar( const QString &key );
    SettingsVariable *svar( const QString &key );
    ConsoleVariable *defaultCvar;
    SettingsVariable *defaultSvar;

    // event
    EventEntry *currentEvent();
    EventEntry *eventForId( int id );

    // lists
    typedef struct ketoList_s {
        QList <TeamEntry*>  teamList;
        QList <TaskEntry*>  taskList;
        QList <LogEntry*>   logList;
        QList <EventEntry*> eventList;
    } ketoList_t;
    ketoList_t base;
    ketoList_t import;
    QList <TaskEntry*>  taskListSorted();
    QList <ConsoleVariable*>  cvarList;
    QList <SettingsVariable*> svarList;

    // misc
    QString transliterate( const QString &path );
    bool isInitialized() const { return this->m_init; }
    DebugLevels debugLevel() const { return this->m_debug; }

    // mem debug
#ifdef APPLET_DEBUG
    int alloc;
    int dealloc;
#endif

public slots:
    // init/shutdown
    void initialize( QObject *parent );
    void setInitialized( bool init = true ) { this->m_init = init; }
    void shutdown( bool ignoreDatabase = false );

    // database
    void touchDatabase( const QString &prefix = "" );

    // debug
    void setDebugLevel( DebugLevels debug ) { this->m_debug = debug; }

    // database related
    void addTeam( const QString &teamName, int members, QTime finishTime, const QString &reviewerName = QString::null, bool lockState = false );
    void addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style = TaskEntry::NoStyle , const QString &description = QString::null );
    void addEvent( const QString &title = QString::null );
    void attachDatabase( const QString &path, Import = LogImport );
    void removeTeam( const QString &teamName );

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
    bool setCurrentEvent( EventEntry *eventPtr );
    void buildEventTTList();
    void showConsole() { this->console->show(); }
    void hideConsole() { this->console->hide(); }

    // combos
    int getFreeComboHandle() const;

private:
    QSettings *settings;
    int changesCounter;
    EventEntry *m_event;
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
    void loadDatabase();
    void unloadDatabase();
    void makePath( const QString &path = QString::null );
    void loadTasks( bool import = false, bool store = false );
    void loadTeams( bool import = false, bool store = false );
    void loadLogs( bool import = false, bool store = false );
    bool loadEvents( bool import = false );
    void removeOrphanedLogs();
    void writeBackup();
};

// flags
Q_DECLARE_OPERATORS_FOR_FLAGS( Main::DebugLevels )

//
// externals
//
extern class Main m;

#endif // MAIN_H
