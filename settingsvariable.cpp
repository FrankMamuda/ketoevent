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

//
// includes
//
#include "settingsvariable.h"
#include "gui_settings.h"
#include "gui_rankings.h"
#include "main.h"

/*
================
construct
================
*/
SettingsVariable::SettingsVariable( const QString &key, SettingsVariable::Types bType, SettingsVariable::Class varClass ) {
    // set data and type
    this->setType( bType );
    this->setClass( varClass );
    this->setKey( key );
}

/*
================
bind
================
*/
void SettingsVariable::bind( QObject *objPtr, QObject *parentPtr ) {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;
    QLineEdit *lPtr;
    QAction *aPtr;

    // set object and parent
    this->objPtr = objPtr;
    this->setParent( parentPtr );

    // failsafe
    if ( this->parent() == NULL || this->objPtr == NULL ) {
        m.error( StrSoftError, QString( this->tr( "unable to bind settings var '%1\n" )).arg( this->key()));
        return;
    }

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

    case Action:
        aPtr = qobject_cast<QAction*>( this->objPtr );
        aPtr->connect( aPtr, SIGNAL( toggled( bool)), this, SLOT( toggled( bool )));
        break;

    default:
        m.error( StrSoftError, this->tr( "unknown type\n" ));
        return;
    }

    // set state to current value
    this->setState();
}

/*
================
unbind
================
*/
void SettingsVariable::unbind() {
    QSpinBox *sPtr;
    QCheckBox *cPtr;
    QTimeEdit *tPtr;
    QLineEdit *lPtr;
    QAction *aPtr;

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

    case Action:
        aPtr = qobject_cast<QAction*>( this->objPtr );
        aPtr->disconnect( aPtr, SIGNAL( toggled( bool )));
        break;

    case NoType:
    default:
        break;
    }

    // reset object
    this->objPtr = NULL;
    this->setParent( NULL );
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
    QAction *aPtr;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
    {
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = m.cvar( this->key())->isEnabled();
        else
            state = m.currentEvent()->record().value( this->key()).toBool();

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
            value = m.cvar( this->key())->integer();
        else
            value = m.currentEvent()->record().value( this->key()).toInt();

        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( value );
    }
        break;

    case TimeEdit:
    {
        QTime time;

        if ( this->varClass() == ConsoleVar )
            time = m.cvar( this->key())->time();
        else
            time = QTime::fromString( m.currentEvent()->record().value( this->key()).toString(), "hh:mm" );

        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->setTime( time );
    }
        break;

    case LineEdit:
    {
        QString text;

        if ( this->varClass() == ConsoleVar )
            text = m.cvar( this->key())->string();
        else
            text = m.currentEvent()->record().value( this->key()).toString();

        lPtr = qobject_cast<QLineEdit*>( this->objPtr );
        lPtr->setText( text );
    }
        break;


    case Action:
    {
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = m.cvar( this->key())->isEnabled();
        else
            state = m.currentEvent()->record().value( this->key()).toBool();

        aPtr = qobject_cast<QAction*>( this->objPtr );
        if ( state )
            aPtr->setChecked( true );
        else
            aPtr->setChecked( false );
    }
        break;

    case NoType:
    default:
        m.error( StrSoftError, this->tr( "unknown type\n" ));
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

    if ( sParent == NULL )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar ) {
        if ( state == Qt::Checked )
            m.cvar( this->key())->setValue( true );
        else
            m.cvar( this->key())->setValue( false );
    } else {
        if ( state == Qt::Checked )
            m.currentEvent()->setValue( this->key(), true );
        else
            m.currentEvent()->setValue( this->key(), false );
    }
}

/*
================
toggled
================
*/
void SettingsVariable::toggled( bool state ) {
    Gui_Dialog *sParent = qobject_cast<Gui_Dialog*>( this->parent());

    if ( sParent == NULL )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar ) {
        if ( state == true )
            m.cvar( this->key())->setValue( true );
        else
            m.cvar( this->key())->setValue( false );
    } else {
        if ( state == true )
            m.currentEvent()->setValue( this->key(), true );
        else
            m.currentEvent()->setValue( this->key(), false );
    }
}

/*
================
integerValueChanged
================
*/
void SettingsVariable::integerValueChanged( int integer ) {
    Gui_Dialog *sParent = qobject_cast<Gui_Dialog*>( this->parent());

    if ( sParent == NULL )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        m.cvar( this->key())->setValue( integer );
    else
        m.currentEvent()->setValue( this->key(), integer );
}

/*
================
timeChanged
================
*/
void SettingsVariable::timeChanged( const QTime &time ) {
    Gui_Dialog *sParent = qobject_cast<Gui_Dialog*>( this->parent());

    if ( sParent == NULL )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        m.cvar( this->key())->setValue( time );
    else
        m.currentEvent()->setValue( this->key(), time.toString( "hh:mm" ));
}

/*
================
textChanged
================
*/
void SettingsVariable::textChanged( const QString &text ) {
    Gui_Dialog *sParent = qobject_cast<Gui_Dialog*>( this->parent());

    if ( sParent == NULL )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        m.cvar( this->key())->setValue( text );
    else
        m.currentEvent()->setValue( this->key(), text );
}
