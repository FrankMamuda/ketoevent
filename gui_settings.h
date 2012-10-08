/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include <QDialog>
#include "gui_settingscvar.h"

//
// namespaces
//
namespace Ui {
    class Gui_Settings;
}

//
// class:Gui_Settings
//
class Gui_Settings : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Settings dialog" )
    Q_PROPERTY( bool cvarsLocked READ cvarsLocked WRITE lockCvars )
    Q_ENUMS( FileDialog )

public:
    enum FileDialog {
        Open = 0,
        Save
    };
    explicit Gui_Settings( QWidget *parent = 0 );
    ~Gui_Settings();
    bool cvarsLocked() const { return this->m_cvarsLocked; }
    void addCvar( pCvar *cvarPtr, pSettingsCvar::Types type, QObject *objPtr ) {
        this->cvarList << new pSettingsCvar( cvarPtr, objPtr, type, qobject_cast<QObject*>( this ));
    }

private slots:
    void lockCvars( bool lock = true ) { this->m_cvarsLocked = lock; }
    void intializeCvars();
    void on_buttonExport_clicked();
    void on_buttonImport_clicked();
    void updateDrunkMode( const QString &, const QString & );

private:
    Ui::Gui_Settings *ui;
    bool m_cvarsLocked;
    QString getFilename( FileDialog );

    // auto cvars
    QList <pSettingsCvar*>cvarList;
};

#endif // GUI_SETTINGS_H
