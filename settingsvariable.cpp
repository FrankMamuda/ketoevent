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
    QLineEdit *lPtr;

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

    case LineEdit:
        lPtr = qobject_cast<QLineEdit*>( this->objPtr );
        lPtr->connect( lPtr, SIGNAL( textChanged( QString )), this, SLOT( textChanged( QString )));
        break;

    default:
        m.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
disconnectVars
================
*/
void SettingsVariable::disconnectVars() {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;
    QLineEdit *lPtr;

    // connect slots
    switch ( this->type()) {
    case CheckBox:
        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        cPtr->disconnect( cPtr, SIGNAL( stateChanged( int )));
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->disconnect( sPtr, SIGNAL( valueChanged( int )));
        break;

    case TimeEdit:
        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->disconnect( tPtr, SIGNAL( timeChanged( QTime )));
        break;

    case LineEdit:
        lPtr = qobject_cast<QLineEdit*>( this->objPtr );
        lPtr->disconnect( lPtr, SIGNAL( textChanged( QString )));
        break;
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
    QLineEdit *lPtr;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
    {
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
    }
        break;

    case SpinBox:
    {
        int value;

        if ( this->varClass() == ConsoleVar )
            value = m.var( this->key())->integer();
        else
            value = m.event->record().value( this->key()).toInt();

        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( value );
    }
        break;

    case TimeEdit:
    {
        QTime time;

        if ( this->varClass() == ConsoleVar )
            time = m.var( this->key())->time();
        else
            time = QTime::fromString( m.event->record().value( this->key()).toString(), "hh:mm" );

        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->setTime( time );
    }
        break;

    case LineEdit:
    {
        QString text;

        if ( this->varClass() == ConsoleVar )
            text = m.var( this->key())->string();
        else
            text = m.event->record().value( this->key()).toString();

        lPtr = qobject_cast<QLineEdit*>( this->objPtr );
        lPtr->setText( text );
    }
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

    if ( this->varClass() == ConsoleVar ) {
        if ( state == Qt::Checked )
            m.var( this->key())->setValue( true );
        else
            m.var( this->key())->setValue( false );
    } else {
        if ( state == Qt::Checked )
            m.event->setValue( this->key(), true );
        else
            m.event->setValue( this->key(), false );
    }
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

    if ( this->varClass() == ConsoleVar )
        m.var( this->key())->setValue( integer );
    else
        m.event->setValue( this->key(), integer );
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

    if ( this->varClass() == ConsoleVar )
        m.var( this->key())->setValue( time );
    else
        m.event->setValue( this->key(), time.toString( "hh:mm" ));
}

/*
================
textChanged
================
*/
void SettingsVariable::textChanged( const QString &text ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        m.var( this->key())->setValue( text );
    else
        m.event->setValue( this->key(), text );
}
