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

//
// includes
//
#include "settingsvariable.h"
#include "gui_settings.h"

/*
================
construct
================
*/
SettingsVariable::SettingsVariable( const QString &key, QObject *bObjPtr, SettingsVariable::Types bType, QObject *parent, SettingsVariable::Class varClass ) {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;

    // set data, type and object
    this->objPtr = bObjPtr;
    this->setType( bType );
    this->setClass( varClass );
    this->setParent( parent );
    this->setKey( key );

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
        m.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
setState
================
*/
void SettingsVariable::setState() {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = m.var( this->key())->isEnabled();
        else
            state = m.event->record().value( this->key()).toBool();

        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        if ( state )
            cPtr->setCheckState( Qt::Checked );
        else
            cPtr->setCheckState( Qt::Unchecked );
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( m.var( this->key())->integer());
        break;

    case TimeEdit:
        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->setTime( m.var( this->key())->time());
        break;

    default:
        m.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
stateChanged
================
*/
void SettingsVariable::stateChanged( int state ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->variablesLocked())
        return;

    if ( state == Qt::Checked )
        m.var( this->key())->setValue( true );
    else
        m.var( this->key())->setValue( false );
}

/*
================
integerValueChanged
================
*/
void SettingsVariable::integerValueChanged( int integer ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->variablesLocked())
        return;

    m.var( this->key())->setValue( integer );
}

/*
================
timeChanged
================
*/
void SettingsVariable::timeChanged( const QTime &time ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->variablesLocked())
        return;

    m.var( this->key())->setValue( time );
}
