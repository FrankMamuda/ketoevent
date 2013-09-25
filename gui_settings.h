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

//
// namespaces
//
namespace Ui {
    class Gui_Settings;
}

//
// class: Gui_Settings
//
class Gui_Settings : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Settings dialog" )
    Q_PROPERTY( bool variablesLocked READ variablesLocked WRITE lockVariables )
    Q_ENUMS( FileDialog )

public:
    enum FileDialog {
        Open = 0,
        Save
    };
    explicit Gui_Settings( QWidget *parent = 0 );
    ~Gui_Settings();
    bool variablesLocked() const { return this->m_variablesLocked; }
    void addVariable( const QString &key, SettingsVariable::Types type, QObject *objPtr ) {
        this->varList << new SettingsVariable( key, objPtr, type, qobject_cast<QObject*>( this ));
    }

private slots:
    void lockVariables( bool lock = true ) { this->m_variablesLocked = lock; }
    void intializeVariables();
    void on_buttonImport_clicked();
    void updateDrunkMode( const QString &, const QString & );

    void on_buttonExportCSV_clicked();

private:
    Ui::Gui_Settings *ui;
    bool m_variablesLocked;

    // auto cvars
    QList <SettingsVariable*>varList;
};

#endif // GUI_SETTINGS_H
