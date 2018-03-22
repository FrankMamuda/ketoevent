/*
 * Copyright (C) 2013-2018 Factory #12
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

//
// includes
//
#include "settingsvariable.h"
#include "settings.h"
#include "rankings.h"
#include "main.h"

/**
 * @brief SettingsVariable::SettingsVariable
 * @param key
 * @param bType
 * @param varClass
 */
SettingsVariable::SettingsVariable( const QString &key, SettingsVariable::Types bType, SettingsVariable::Class varClass ) {
    // set data and type
    this->setType( bType );
    this->setClass( varClass );
    this->setKey( key );
}

/**
 * @brief SettingsVariable::bind
 * @param objPtr
 * @param parentPtr
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
    if ( this->parent() == nullptr || this->objPtr == nullptr ) {
        Common::error( StrSoftError, this->tr( "unable to bind settings variable \"%1\"\n" ).arg( this->key()));
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
        Common::error( StrSoftError, this->tr( "unknown type\n" ));
        return;
    }

    // set state to current value
    this->setState();
}

/**
 * @brief SettingsVariable::unbind
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
    this->objPtr = nullptr;
    this->setParent( nullptr );
}

/**
 * @brief SettingsVariable::setState
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
            state = Variable::instance()->isEnabled( this->key());
        else
            state = Event::active()->record().value( this->key()).toBool();

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
            value = Variable::instance()->integer( this->key());
        else
            value = Event::active()->record().value( this->key()).toInt();

        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( value );
    }
        break;

    case TimeEdit:
    {
        QTime time;

        if ( this->varClass() == ConsoleVar )
            time = Variable::instance()->value<QTime>( this->key());
        else
            time = QTime::fromString( Event::active()->record().value( this->key()).toString(), "hh:mm" );

        tPtr = qobject_cast<QTimeEdit*>( this->objPtr );
        tPtr->setTime( time );
    }
        break;

    case LineEdit:
    {
        QString text;

        if ( this->varClass() == ConsoleVar )
            text = Variable::instance()->string( this->key());
        else
            text = Event::active()->record().value( this->key()).toString();

        lPtr = qobject_cast<QLineEdit*>( this->objPtr );
        lPtr->setText( text );
    }
        break;


    case Action:
    {
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = Variable::instance()->isEnabled( this->key());
        else
            state = Event::active()->record().value( this->key()).toBool();

        aPtr = qobject_cast<QAction*>( this->objPtr );
        if ( state )
            aPtr->setChecked( true );
        else
            aPtr->setChecked( false );
    }
        break;

    case NoType:
    default:
        Common::error( StrSoftError, this->tr( "unknown settings variable type" ));
        return;
    }
}

/**
 * @brief SettingsVariable::stateChanged
 * @param state
 */
void SettingsVariable::stateChanged( int state ) {
    Settings *sParent = qobject_cast<Settings*>( this->parent());

    if ( sParent == nullptr )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        Variable::instance()->setValue( this->key(), state == Qt::Checked ? true : false );
    else
        Event::active()->setValue( this->key(), state == Qt::Checked ? true : false );

}

/**
 * @brief SettingsVariable::toggled
 * @param state
 */
void SettingsVariable::toggled( bool state ) {
    Dialog *sParent = qobject_cast<Dialog*>( this->parent());

    if ( sParent == nullptr )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        Variable::instance()->setValue( this->key(), state );
    else
        Event::active()->setValue( this->key(), state );
}

/**
 * @brief SettingsVariable::integerValueChanged
 * @param integer
 */
void SettingsVariable::integerValueChanged( int integer ) {
    Dialog *sParent = qobject_cast<Dialog*>( this->parent());

    if ( sParent == nullptr )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        Variable::instance()->setValue( this->key(), integer );
    else
        Event::active()->setValue( this->key(), integer );
}

/**
 * @brief SettingsVariable::timeChanged
 * @param time
 */
void SettingsVariable::timeChanged( const QTime &time ) {
    Dialog *sParent = qobject_cast<Dialog*>( this->parent());

    if ( sParent == nullptr )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        Variable::instance()->setValue( this->key(), time );
    else
        Event::active()->setValue( this->key(), time.toString( "hh:mm" ));
}

/**
 * @brief SettingsVariable::textChanged
 * @param text
 */
void SettingsVariable::textChanged( const QString &text ) {
    Dialog *sParent = qobject_cast<Dialog*>( this->parent());

    if ( sParent == nullptr )
        return;

    if ( sParent->variablesLocked())
        return;

    if ( this->varClass() == ConsoleVar )
        Variable::instance()->setValue( this->key(), text );
    else
        Event::active()->setValue( this->key(), text );
}

/**
 * @brief Variable::add
 * @param varPtr
 */
void SettingsVariable::add( const QString &key, SettingsVariable::Types type, SettingsVariable::Class varClass ) {
    // avoid duplicates
    if ( SettingsVariable::find( key ) != nullptr )
        return;

    Main::instance()->svarList << new SettingsVariable( key, type, varClass );
}

/**
 * @brief find
 * @param key
 * @return
 */
SettingsVariable *SettingsVariable::find( const QString &key ) {
    foreach ( SettingsVariable *varPtr, Main::instance()->svarList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return nullptr;
}
