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

#ifndef GUI_EVENT_H
#define GUI_EVENT_H

//
// includes
//
#include "gui_settingsdialog.h"


//
// namespace: Ui
//
namespace Ui {
class Gui_Event;
}

//
// class: Gui_Event
//
class Gui_Event : public Gui_SettingsDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Event handling dialog" )

public:
    explicit Gui_Event( QWidget *parent = 0 );
    ~Gui_Event();

public slots:
    void fillEvents();

private slots:
    void bindVars();
    void on_buttonClose_clicked();
    void on_eventCombo_currentIndexChanged( int index );
    void on_buttonAdd_clicked();
    void validate();
    void on_buttonRemove_clicked();

private:
    Ui::Gui_Event *ui;
};

#endif // GUI_EVENT_H
