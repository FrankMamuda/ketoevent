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

#ifndef GUI_ADDEVENT_H
#define GUI_ADDEVENT_H

//
// includes
//
#include <QDialog>

//
// namespace: Ui
//
namespace Ui {
class Gui_AddEvent;
}

//
// class: AddEvent (ui)
//
class Gui_AddEvent : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Event addition dialog" )

public:
    explicit Gui_AddEvent( QWidget *parent = 0 );
    ~Gui_AddEvent();

private slots:
    void on_addButton_clicked();

private:
    Ui::Gui_AddEvent *ui;
};

#endif // GUI_ADDEVENT_H
