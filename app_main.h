/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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

#ifndef APP_MAIN_H
#define APP_MAIN_H

//
// includes
//
#include "sys_shared.h"
#include "app_teamentry.h"
#include "app_taskentry.h"
#include "sys_cvar.h"

//
// defines
//
#define FORCE_LATVIAN

//
// class:App_Main
//
class App_Main : public QObject {
    Q_OBJECT
    Q_ENUMS( ListTypes )

public:
    enum ListTypes {
        NoType = -1,
        Teams,
        Tasks
    };
    QList <App_TaskEntry*> taskList;
    QList <App_TeamEntry*> teamList;
    App_TaskEntry* taskForHash( const QString &hash );
    void loadTeams();
    void loadTasks();
    bool addTeam( App_TeamEntry *, bool = true, bool = false );
    void removeTeam( App_TeamEntry * );
    bool addTask( App_TaskEntry *, bool = true );
    void removeTask( App_TaskEntry * );
    App_TeamEntry *findTeamByName( const QString & );
    App_TaskEntry *findTaskByName( const QString & );
    template <class T>
    bool parseXML( const QString &filename, const QString &buffer, T *handlerPtr );

signals:
    void teamListChanged();
    void taskListChanged();

public slots:
    void shutdown();
    void sort( ListTypes type );
    void updateTeamList();
    void updateTaskList();
};

//
// externals
//
extern class App_Main m;

#endif // APP_MAIN_H
