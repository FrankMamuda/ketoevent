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

#ifndef GUI_SETTINGSDIALOG_H
#define GUI_SETTINGSDIALOG_H

//
// includes
//
#include <QDialog>
#include "settingsvariable.h"
#include "main.h"

//
// inherited by Gui_Event and Gui_Settings
//

//
// class: Settings Dialog (gui)
//
class Gui_SettingsDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY( bool variablesLocked READ variablesLocked WRITE lockVariables )
    Q_CLASSINFO( "description", "Settings base class" )

public:
    Gui_SettingsDialog( QWidget *parent ) : QDialog( parent ) {}
    bool variablesLocked() const { return this->m_variablesLocked; }

public slots:
    void lockVariables( bool lock = true ) { this->m_variablesLocked = lock; }
    void unbindVars() { if ( !m.isInitialized()) return; foreach ( SettingsVariable* svar, this->svars ) svar->unbind(); svars.clear(); }
    void bindVariable( const QString &key, QObject *object ) { this->svars << m.svar( key ); m.svar( key )->bind( object, qobject_cast<QObject*>( this )); }
    void updateVars() { foreach ( SettingsVariable *svar, this->svars ) svar->setState(); }

private:
    bool m_variablesLocked;
    QList <SettingsVariable*> svars;
};

#endif // GUI_SETTINGSDIALOG_H
