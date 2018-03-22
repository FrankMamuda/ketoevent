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

#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSpacerItem>
#include "task.h"
#include "log.h"
#include "team.h"

//
// defines
//
#define UI_INFO_BUTTON_ENABLED

/**
 * @brief The TaskWidget class
 */
class TaskWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY( Log *log READ log WRITE setLog RESET resetLog )
    Q_PROPERTY( Task *task READ task WRITE setTask )
    Q_PROPERTY( Team *team READ team WRITE setTeam RESET resetTeam )
    Q_PROPERTY( bool active READ isActive WRITE setActive )
    Q_CLASSINFO( "description", "Task logging widget" )

public:
    explicit TaskWidget( Task *parentPtr );
    ~TaskWidget();
    Log *log() const { return this->m_log; }
    Task *task() const { return this->m_task; }
    Team *team() const { return this->m_team; }
    bool hasLog() const { return this->log() != nullptr; }
    bool hasCombo() const { if ( this->hasLog()) { if ( this->log()->comboId() != -1 ) return true; } return false; }
    bool hasTeam() const { return this->team() != nullptr; }
    bool hasTask() const { return this->task() != nullptr; }
    static int getRelativeComboId( int comboId, int teamId );
    bool isActive() const { return this->m_active; }
    enum ViewState {
        NoState = -1,
        LogView,
        Combine
    };

private slots:
    void setTask( Task *taskPtr ) { this->m_task = taskPtr; }
    void resetLog();
    void setActive( bool active = true ) { this->m_active = active; }
#ifdef UI_INFO_BUTTON_ENABLED
    void displayInfo();
#endif

public slots:
    void setTeam( Team *teamPtr );
    void resetTeam();
    void saveLog();
    void setLog( Log *logPtr, bool fromDatabase = false );
    void toggleCombo( bool );
    void comboIdChanged();
    void toggleViewState( ViewState );

public:
    QHBoxLayout *taskLayout;
    QLabel *taskName;
    QLabel *comboIcon;
    QCheckBox *check;
    QPushButton *combo;
    QSpinBox *multi;
    QSpacerItem *spacer;
    Log *m_log;
    Task *m_task;
    Team *m_team;
    bool m_changed;
    bool m_active;
#ifdef UI_INFO_BUTTON_ENABLED
    QPushButton *info;
#endif
};
