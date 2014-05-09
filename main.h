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
#include <QSqlDatabase>
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

// error message macros
#define StrMsg ClassFunc
#define StrFatalError Main::FatalError, ClassFunc
#define StrSoftError Main::SoftError, ClassFunc
#define StrWarn QObject::trUtf8( "WARNING:" ) + ClassFunc

// pointer macro
#define GetPtr( type, name, container ) type name = qobject_cast<type>( container );
#define TestPtr( name ) if ( name == NULL )

//
// namespace: Common
//
namespace Common {
    const static unsigned int API = 6;
    const static unsigned int MinimumAPI = 6;
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
    Q_CLASSINFO( "description", "Applet main class" )

public:
    // sorting types
    enum ListTypes {
        NoType = -1,
        Teams,
        Tasks
    };

    // error types
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

    // database related
    LogEntry *addLog( int taskId, int teamId, int value = 0, int comboId = -1 );
    LogEntry *logForId( int id );
    LogEntry *logForIds( int teamId, int taskId );
    TaskEntry *taskForId( int id );
    TaskEntry *taskForName( const QString &name );
    TeamEntry *teamForId( int id );
    TeamEntry *teamForName( const QString &name );
    QString path;

    // console/settings variables
    ConsoleVariable *cvar( const QString &key );
    SettingsVariable *svar( const QString &key );
    ConsoleVariable *defaultCvar;
    SettingsVariable *defaultSvar;

    // event
    EventEntry *currentEvent();
    EventEntry *eventForId( int id );

    // lists
    QList <TeamEntry*>  teamList;
    QList <TaskEntry*>  taskList;
    QList <TaskEntry*>  taskListSorted();
    QList <LogEntry*>   logList;
    QList <EventEntry*> eventList;
    QList <ConsoleVariable*>  cvarList;
    QList <SettingsVariable*> svarList;

    // misc
    QString transliterate( const QString &path );
    bool isInitialized() const { return this->m_init; }

    // combination statistics
    typedef struct stats_s {
        int points;
        int combos;
        int total;

        /* expanded to support statistics imports */
        int tasks;
        int timeOnTrack;
    } stats_t;
    stats_t getComboStats( int id ) const;

public slots:
    // init/shutdown
    void initialize( QObject *parent );
    void setInitialized( bool init = true ) { this->m_init = init; }
    void shutdown( bool ignoreDatabase = false );

    // database related
    void addTeam( const QString &teamName, int members, QTime finishTime, bool lockState = false );
    void addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style = TaskEntry::NoStyle );
    void addEvent( const QString &title = QString::null );

    // console io
    void error( ErrorTypes type, const QString &msg );
    void print( const QString &msg );

    // misc
    void sort( ListTypes type );
    void update();
    void initConsole();
    void clearEvent();// { this->teamList.clear(); this->taskList.clear(); this->taskListSorted().clear(); this->logList.clear(); this->eventList.clear(); }
    void reloadDatabase( const QString &path ) { this->unloadDatabase(); this->makePath( path ); this->loadDatabase(); }
    bool setCurrentEvent( EventEntry *eventPtr );

    // combos
    int getFreeComboHandle() const;

private:
    QSettings *settings;
    int changesCounter;
    EventEntry *m_event;
    bool m_init;
    Gui_Console *console;

private slots:
    // console/settings variables
    void addCvar( ConsoleVariable *varPtr );
    void addSvar( const QString &key, SettingsVariable::Types type, SettingsVariable::Class varClass );

    // database related
    void loadDatabase();
    void unloadDatabase();
    void makePath( const QString &path = QString::null );
    void loadTasks();
    void loadTeams();
    void loadLogs();
    void loadEvents();
    void removeTeam( const QString &teamName );
    void removeOrphanedLogs();
    void writeBackup();
};

//
// externals
//
extern class Main m;

#endif // MAIN_H
