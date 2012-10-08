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

#ifndef GUI_SETTINGSCVAR_H
#define GUI_SETTINGSCVAR_H

//
// includes
//
#include "sys_cvar.h"
#include "sys_common.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QTimeEdit>

//
// class:pSettingsCvar
//
class pSettingsCvar : public QObject {
    Q_OBJECT
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_ENUMS( Types )

public:
    // currently supported types
    enum Types {
        CheckBox = 0,
        SpinBox,
        TimeEdit
    };
    Types type () const { return this->m_type; }

    // constructor
    pSettingsCvar( pCvar *bCvarPtr, QObject *bObjPtr, pSettingsCvar::Types bType, QObject *parent );

    // set initial values from cvars
    void setState();

    // save value
    void save();

public slots:
    void setType( Types bType ) { this->m_type = bType; }

    // checkBox
    void stateChanged( int state );

    // spinBox
    void integerValueChanged( int integer );

    // timeEdit
    void timeChanged( const QTime &time );

private:
    Types m_type;

    // pointers to cvar and corresponding object
    pCvar *cvarPtr;
    QObject *objPtr;
};

#endif // GUI_SETTINGSCVAR_H
