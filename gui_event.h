/*
===========================================================================
Copyright (C) 2013-2015 Avotu Briezhaudzetava

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
#include "gui_dialog.h"
#include "ui_gui_event.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Event;
}

//
// class: Gui_Event
//
class Gui_Event : public Gui_Dialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Event handling dialog" )
    Q_PROPERTY( bool import READ importPerformed WRITE setImported )

public:
    explicit Gui_Event( QWidget *parent = 0 );
    ~Gui_Event();

public slots:
    void fillEvents();
    bool importPerformed() const { return m_import; }

private slots:
    void bindVars();
    void on_buttonClose_clicked();
    void on_eventCombo_currentIndexChanged( int index );
    void validate();
    void setImported( bool import = true ) { this->m_import = import; }
    void on_actionAddEvent_triggered();
    void on_actionRemoveEvent_triggered();
    void on_actionImportLogs_triggered();
    void on_actionImportTasks_triggered();
    void on_actionExportEvent_triggered();
    void on_actionExportTasks_triggered();
    // FIXME: USE HOMEDIR FOR MAC

    void on_actionRename_triggered();

private:
    Ui::Gui_Event *ui;
    bool m_import;
};

#endif // GUI_EVENT_H

