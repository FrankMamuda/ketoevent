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

#ifndef GUI_TASKWIDGET_H
#define GUI_TASKWIDGET_H

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
#include "taskentry.h"
#include "logentry.h"
#include "teamentry.h"

//
// class: TaskWidget
//
class TaskWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY( LogEntry *log READ log WRITE setLog RESET resetLog )
    Q_PROPERTY( TaskEntry *task READ task WRITE setTask )
    Q_PROPERTY( TeamEntry *team READ team WRITE setTeam RESET resetTeam )
    Q_PROPERTY( LogEntry::Combos comboState READ comboState WRITE setComboState )

public:
    explicit TaskWidget( TaskEntry *parentPtr );
    ~TaskWidget();
    LogEntry *log() const { return this->m_log; }
    TaskEntry *task() const { return this->m_task; }
    TeamEntry *team() const { return this->m_team; }
    LogEntry::Combos comboState() const { return this->m_comboState; }
    bool hasLog() const { return this->log() != NULL; }
    bool hasTeam() const { return this->team() != NULL; }
    bool hasTask() const { return this->task() != NULL; }

private slots:
    void setTask( TaskEntry *taskPtr ) { this->m_task = taskPtr; }
    void setComboState( LogEntry::Combos combo );
    void toggleCombo( bool checked );
    void resetLog();

public slots:
    void setTeam( TeamEntry *teamPtr );
    void resetTeam();
    void saveLog();
    void setLog( LogEntry *logPtr, bool fromDatabase = false );

//private:
// TODO: disabler function
public:
    QGridLayout *grid;
    QLabel *taskName;
    QCheckBox *check;
    QPushButton *combo;
    QSpinBox *multi;
    LogEntry *m_log;
    TaskEntry *m_task;
    TeamEntry *m_team;
    LogEntry::Combos m_comboState;
    bool m_changed;
    bool m_active;
};

#endif // GUI_TASKWIDGET_H
