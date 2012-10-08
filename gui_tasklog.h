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

#ifndef GUI_TASKLOG_H
#define GUI_TASKLOG_H

//
// includes
//
#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
//#include <QLineEdit>
#include "app_tasklog.h"
#include "app_taskentry.h"
#include "app_teamentry.h"

//
// class:Gui_TaskLog
//
class Gui_TaskLog : public QWidget {
    Q_OBJECT
    Q_PROPERTY( App_TaskLog *log READ log WRITE setLog RESET resetLog )
    Q_PROPERTY( App_TaskEntry *task READ task WRITE setTask )
    Q_PROPERTY( App_TeamEntry *team READ team WRITE setTeam RESET resetTeam )
    Q_PROPERTY( App_TaskLog::Combos comboState READ comboState WRITE setComboState )
    Q_PROPERTY( bool changed READ changed WRITE setChanged )

public:
    explicit Gui_TaskLog( App_TaskEntry *parentPtr );
    ~Gui_TaskLog();
    App_TaskLog *log() const { return this->m_log; }
    App_TaskEntry *task() const { return this->m_task; }
    App_TeamEntry *team() const { return this->m_team; }
    App_TaskLog::Combos comboState() const { return this->m_comboState; }
    bool hasLog() const { return this->log() != NULL; }
    bool hasTeam() const { return this->team() != NULL; }
    bool hasTask() const { return this->task() != NULL; }
    bool changed() const { return this->m_changed; }

private slots:
    void setTask( App_TaskEntry *entryPtr ) { this->m_task = entryPtr; }
    void setComboState( App_TaskLog::Combos combo );
    void toggleCombo();
    void update();
    void setLog( App_TaskLog *logPtr );
    void resetLog();
    void setChanged( bool changed = true ) { this->m_changed = changed; }

signals:
    void dataChanged();

public slots:
    void setTeam( App_TeamEntry *teamPtr );
    void resetTeam();
    void saveLog();

private:
    QGridLayout *grid;
    QLabel *taskName;
    QCheckBox *check;
    QPushButton *combo;
    QSpinBox *multi;
    //QLineEdit *points;
    App_TaskLog *m_log;
    App_TaskEntry *m_task;
    App_TeamEntry *m_team;
    App_TaskLog::Combos m_comboState;
    bool m_changed;
};

#endif // GUI_TASKLOG_H
