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

#ifndef GUI_COMBOS_H
#define GUI_COMBOS_H

//
// includes
//
#include <QDialog>
#include "gui_combomodel.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Combos;
}

//
// classes
//
class Gui_ComboModel;

//
// class: Combos (gui)
//
class Gui_Combos : public QDialog {
    Q_OBJECT

public:
    explicit Gui_Combos( QWidget *parent = 0 );
    ~Gui_Combos();
    QList <LogEntry *>logListSorted;
    int currentTeamId() const { return this->m_currentTeamId; }

private slots:
    void fillTeams();
    void on_buttonClose_clicked();
    void teamIndexChanged( int );

private:
    Ui::Gui_Combos *ui;
    Gui_ComboModel *comboModelPtr;
    int m_currentTeamId;
};

#endif // GUI_COMBOS_H
