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

#ifndef GUI_SETTINGS_H
#define GUI_SETTINGS_H

//
// includes
//
#include <QtGlobal>
#include <QDialog>
#include "settingsvariable.h"
#include "gui_settingsdialog.h"

//
// namespaces
//
namespace Ui {
    class Gui_Settings;
}

//
// class: Gui_Settings
//
class Gui_Settings : public Gui_SettingsDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Settings dialog" )
    Q_ENUMS( FileDialog )

public:
    enum FileDialog {
        Open = 0,
        Save
    };
    explicit Gui_Settings( QWidget *parent = 0 );
    ~Gui_Settings();

private slots:
    void bindVars();
    void on_buttonImport_clicked();
    void on_buttonExportCSV_clicked();
    void on_buttonExport_clicked();
    void on_backupPerform_stateChanged(int arg1);

private:
    Ui::Gui_Settings *ui;
};

#endif // GUI_SETTINGS_H