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

//
// namespace: Common
//
namespace Common {
    const static unsigned int API = 2;
    const static unsigned int MinimumAPI = 1;
    const static unsigned int defaultMinMembers = 1;
    const static unsigned int defaultMaxMembers = 3;
    const static unsigned int defaultSingleCombo = 1;
    const static unsigned int defaultDoubleCombo = 3;
    const static unsigned int defaultTripleCombo = 5;
    const static unsigned int defaultPenaltyPoints = 5;
    const static QString defaultStartTime( "10:00" );
    const static QString defaultFinishTime( "15:00" );
    const static QString defaultFinalTime( "15:30" );
}

//
// class:Main
//
class Main : public QObject {
    Q_OBJECT
    Q_ENUMS( ListTypes )
    Q_ENUMS( ErrorTypes )

public:
    enum ListTypes {
        NoType = -1,
        Teams,
        Tasks
    };
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

    // lists
    QList <TeamEntry*> teamList;
    QList <TaskEntry*> taskList;
    QList<TaskEntry*> taskListSorted();
    QList <LogEntry*> logList;
    QList <ConsoleVariable*> varList;

    // other funcs
    LogEntry *logForId( int id );
    LogEntry *logForIds( int teamId, int taskId );
    TaskEntry *taskForId( int id );
    TaskEntry *taskForName( const QString &name );
    TeamEntry *teamForId( int id );
    TeamEntry *teamForName( const QString &name );
    QString transliterate( const QString &path );
    ConsoleVariable *var( const QString &key );
    void addVariable( ConsoleVariable *varPtr );
    void addTeam( const QString &teamName, int members, QTime finishTime, bool lockState = false );
    void addTask( const QString &taskName, int points, int multi, TaskEntry::Types type, TaskEntry::Styles style = TaskEntry::NoStyle );
    LogEntry *addLog( int taskId, int teamId, int value = 0, LogEntry::Combos combo = LogEntry::NoCombo );
    QString path;
    QString databasePath;

    // event
    EventEntry *event;

public slots:
    void initialize();
    void shutdown( bool ignoreDatabase = false );
    void loadDatabase();
    void importDatabase( const QString &path );
    void loadTasks();
    void loadTeams();
    void loadLogs();
    void loadEvent();
    void sort( ListTypes type );
    void print( const QString &msg );
    void error( ErrorTypes type, const QString &msg );
    void update();
    void addEvent();

private:
    QSettings *settings;
    void writeBackup();
    int changesCounter;

private slots:
    void deleteOrphanedLogs();
    void removeTeam( const QString &teamName );
};

//
// externals
//
extern class Main m;

#endif // MAIN_H
