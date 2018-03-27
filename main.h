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

#pragma once

//
// includes
//
#include <QObject>
#include <QList>
#include <QTime>
#include "team.h"
#include "task.h"
#include "combo.h"
#include "event.h"
#include "variable.h"
#include "settingsvariable.h"
#include "console.h"
#include "database.h"
#include "common.h"
#include "singleton.h"

//
// defines
//
#define FORCE_LATVIAN

// quickly print to DEBUG
#define QMD( msg ) Common::print( StrMsg + QString( "%1\n" ).arg( msg ), Main::Debug );
#define QMD1( msg, arg1 ) Common::print( StrMsg + QString( "%1 %2\n" ).arg( msg ).arg( arg1 ), Main::Debug );
#define QMD2( msg, arg1, arg2 ) Common::print( StrMsg + QString( "%1 %2 %3\n" ).arg( msg ).arg( arg1 ).arg( arg2 ), Main::Debug );
#define QMD3( msg, arg1, arg2, arg3 ) Common::print( StrMsg + QString( "%1 %2 %3 %4\n" ).arg( msg ).arg( arg1 ).arg( arg2 ).arg( arg3 ), Main::Debug );

//
// namespace: KetoEvent
//
namespace KetoEvent {
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
const static QString comboString( QT_TR_NOOP( "[Imported combos]" ));
const static QString comboDescription( QT_TR_NOOP( "Sum of imported combo points" ));
}

//
// classes
//
#ifdef APPLET_DEBUG
class Console;
#endif
class Database;
class TeamListModel;

/**
 * @brief The Main class
 */
class Main : public QObject {
    Q_OBJECT
    Q_ENUMS( ListTypes )
    Q_PROPERTY( bool initialised READ isInitialised WRITE setInitialised )
    Q_FLAGS( DebugLevels )
    Q_CLASSINFO( "description", "Applet main class" )

public:
    // sorting types
    enum ListTypes {
        NoType = -1,
        Teams,
        Tasks
    };

    // database related
    QString path;

    // lists
    QList <Team*>  teamList;
    QList <Task*>  taskList;
    QList <Log*>   logList;
    QList <Event*> eventList;
    QList <Task*>  taskListSorted();
    QList <Team*>  teamListSorted();
    QList <SettingsVariable*> svarList;

    // misc
    QString transliterate( const QString &path ) const;
    bool isInitialised() const { return this->m_init; }
    Common::DebugLevels debugLevel() const { return this->m_debug; }
    Event *activeEvent;

    // mem debug
#ifdef APPLET_DEBUG
    int alloc;
    int dealloc;
    Console *console;
#endif

    // constructor/destructor/instance
    ~Main();
    static Main *instance() { return Singleton<Main>::instance( Main::createInstance ); }

    TeamListModel *teamModel;

public slots:
    // init/shutdown
    bool initialise( QObject *parent );
    void setInitialised( bool init = true ) { this->m_init = init; }
    void shutdown( bool ignoreDatabase = false );

    // debug
    void setDebugLevel( Common::DebugLevels debug ) { this->m_debug = debug; }

    // misc
    void sort( ListTypes type );
    void update();
    void clearEvent();

private:
    int changesCounter;
    bool m_init;
    Common::DebugLevels m_debug;

    // constructor/destructor/instance
    Main( QObject *parent = nullptr );
    static Main *createInstance() { return new Main(); }
};

// flags
Q_DECLARE_OPERATORS_FOR_FLAGS( Common::DebugLevels )
