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

//
// includes
//
#include "gui_settingscvar.h"
#include "gui_settings.h"

/*
================
construct
================
*/
pSettingsCvar::pSettingsCvar( pCvar *bCvarPtr, QObject *bObjPtr, pSettingsCvar::Types bType, QObject *parent ) {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;

    // set data, type and object
    this->objPtr = bObjPtr;
    this->setType( bType );
    this->setParent( parent );
    this->cvarPtr = bCvarPtr;

    // connect slots
    switch ( this->type()) {
    case CheckBox:
        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        cPtr->connect( cPtr, SIGNAL( stateChanged( int )), this, SLOT( stateChanged( int )));
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->connect( sPtr, SIGNAL( valueChanged( int )), this, SLOT( integerValueChanged( int )));
        break;

    case TimeEdit:
        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->connect( tPtr, SIGNAL( timeChanged( QTime )), this, SLOT( timeChanged( QTime )));
        break;

    default:
        com.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
setState
================
*/
void pSettingsCvar::setState() {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        if ( this->cvarPtr->integer())
            cPtr->setCheckState( Qt::Checked );
        else
            cPtr->setCheckState( Qt::Unchecked );
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( this->cvarPtr->integer());
        break;

    case TimeEdit:
        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->setTime( this->cvarPtr->time());
        break;

    default:
        com.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
stateChanged
================
*/
void pSettingsCvar::stateChanged( int state ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->cvarPtr->set( true );
    else
        this->cvarPtr->set( false );
}

/*
================
integerValueChanged
================
*/
void pSettingsCvar::integerValueChanged( int integer ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->cvarsLocked())
        return;

    this->cvarPtr->set( integer );
}

/*
================
timeChanged
================
*/
void pSettingsCvar::timeChanged( const QTime &time ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->cvarsLocked())
        return;

    this->cvarPtr->set( QString( "%1:%2" ).arg( time.hour()).arg( time.minute()));
}
