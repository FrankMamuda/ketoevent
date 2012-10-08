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

#ifndef APP_TASKLOG_H
#define APP_TASKLOG_H

//
// includes
//
#include "sys_shared.h"
#include "app_taskentry.h"

//
// class:App_TaskLog
//
class App_TaskLog : public QObject {
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( App_TaskEntry *task READ task WRITE setTask )
    Q_ENUMS( Combos )

public:
    enum Combos {
        NoCombo = 0,
        Single = 1,
        Double = 2,
        Triple = 3
    };
    explicit App_TaskLog( int value = 0, Combos combo = NoCombo, App_TaskEntry *taskPtr = NULL );
    int value() const { return this->m_value; }
    bool check() const { return static_cast<bool>( this->m_value ); }
    Combos combo() const { return this->m_combo; }
    int comboPoints() const;
    App_TaskEntry *task() const { return this->m_task; }
    int calculate() { if ( this->task() != NULL ) return task()->calculate( qobject_cast<QObject*>( this )); return 0; }
    QString generateXMLTag();

public slots:
    void setValue( int value ) { if ( value < 0 ) m_value = 0; this->m_value = value; }
    void setCombo( Combos combo ) { this->m_combo = combo; }
    void setTask( App_TaskEntry *taskPtr ) { this->m_task = taskPtr; }

private:
    App_TaskEntry *m_task;
    int m_value;
    Combos m_combo;
};

#endif // APP_TASKLOG_H
