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

#pragma once

//
// includes
//
#include <QDialog>
#include "combomodel.h"
#include "log.h"

//
// namespace: Ui
//
namespace Ui {
class ComboDialog;
}

//
// classes
//
class ComboModel;

/**
 * @brief The ComboDialog class
 */
class ComboDialog : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Combination dialog" )
    Q_PROPERTY( int currentTeamIndex READ currentTeamIndex WRITE setCurrentTeamIndex )

public:
    explicit ComboDialog( QWidget *parent = 0 );
    ~ComboDialog();
    QList <Log *>logListSorted;
    int currentTeamIndex() const { return this->m_currentTeamIndex; }

private slots:
    void fillTeams();
    void currentTeamIndexChanged( int );
    void setCurrentTeamIndex( int index ) { this->m_currentTeamIndex = index; }

private:
    Ui::ComboDialog *ui;
    ComboModel *comboModelPtr;
    int m_currentTeamIndex;
};
