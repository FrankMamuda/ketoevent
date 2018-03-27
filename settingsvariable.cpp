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
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QTimeEdit *timeEdit;
    QLineEdit *lineEdit;
    QAction *action;

    // set object and parent
    this->object = objPtr;
    this->setParent( parentPtr );

    // failsafe
    if ( this->parent() == nullptr || this->object == nullptr ) {
        Common::error( StrSoftError, this->tr( "unable to bind settings variable \"%1\"\n" ).arg( this->key()));
        return;
    }

    // connect slots
    switch ( this->type()) {
    case CheckBox:
        checkBox = qobject_cast<QCheckBox*>( this->object );
        checkBox->connect( checkBox, SIGNAL( stateChanged( int )), this, SLOT( stateChanged( int )));
        break;

    case SpinBox:
        spinBox = qobject_cast<QSpinBox*>( this->object );
        spinBox->connect( spinBox, SIGNAL( valueChanged( int )), this, SLOT( integerValueChanged( int )));
        break;

    case TimeEdit:
        timeEdit = qobject_cast<QTimeEdit*>( this->object );
        timeEdit->connect( timeEdit, SIGNAL( timeChanged( QTime )), this, SLOT( timeChanged( QTime )));
        break;

    case LineEdit:
        lineEdit = qobject_cast<QLineEdit*>( this->object );
        lineEdit->connect( lineEdit, SIGNAL( textChanged( QString )), this, SLOT( textChanged( QString )));
        break;

    case Action:
        action = qobject_cast<QAction*>( this->object );
        action->connect( action, SIGNAL( toggled( bool)), this, SLOT( toggled( bool )));
        break;

    case NoType:
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
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QTimeEdit *timeEdit;
    QLineEdit *lineEdit;
    QAction *action;

    // connect slots
    switch ( this->type()) {
    case CheckBox:
        checkBox = qobject_cast<QCheckBox*>( this->object );
        checkBox->disconnect( checkBox, SIGNAL( stateChanged( int )));
        break;

    case SpinBox:
        spinBox = qobject_cast<QSpinBox*>( this->object );
        spinBox->disconnect( spinBox, SIGNAL( valueChanged( int )));
        break;

    case TimeEdit:
        timeEdit = qobject_cast<QTimeEdit*>( this->object );
        timeEdit->disconnect( timeEdit, SIGNAL( timeChanged( QTime )));
        break;

    case LineEdit:
        lineEdit = qobject_cast<QLineEdit*>( this->object );
        lineEdit->disconnect( lineEdit, SIGNAL( textChanged( QString )));
        break;

    case Action:
        action = qobject_cast<QAction*>( this->object );
        action->disconnect( action, SIGNAL( toggled( bool )));
        break;

    case NoType:
        break;
    }

    // reset object
    this->object = nullptr;
    this->setParent( nullptr );
}

/**
 * @brief SettingsVariable::setState
 */
void SettingsVariable::setState() {
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QTimeEdit *timeEdit;
    QLineEdit *lineEdit;
    QAction *action;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
    {
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = Variable::instance()->isEnabled( this->key());
        else
            state = Event::active()->record().value( this->key()).toBool();

        checkBox = qobject_cast<QCheckBox*>( this->object );
        if ( state )
            checkBox->setCheckState( Qt::Checked );
        else
            checkBox->setCheckState( Qt::Unchecked );
    }
        break;

    case SpinBox:
    {
        int value;

        if ( this->varClass() == ConsoleVar )
            value = Variable::instance()->integer( this->key());
        else
            value = Event::active()->record().value( this->key()).toInt();

        spinBox = qobject_cast<QSpinBox*>( this->object );
        spinBox->setValue( value );
    }
        break;

    case TimeEdit:
    {
        QTime time;

        if ( this->varClass() == ConsoleVar )
            time = Variable::instance()->value<QTime>( this->key());
        else
            time = QTime::fromString( Event::active()->record().value( this->key()).toString(), "hh:mm" );

        timeEdit = qobject_cast<QTimeEdit*>( this->object );
        timeEdit->setTime( time );
    }
        break;

    case LineEdit:
    {
        QString text;

        if ( this->varClass() == ConsoleVar )
            text = Variable::instance()->string( this->key());
        else
            text = Event::active()->record().value( this->key()).toString();

        lineEdit = qobject_cast<QLineEdit*>( this->object );
        lineEdit->setText( text );
    }
        break;


    case Action:
    {
        bool state;

        if ( this->varClass() == ConsoleVar )
            state = Variable::instance()->isEnabled( this->key());
        else
            state = Event::active()->record().value( this->key()).toBool();

        action = qobject_cast<QAction*>( this->object );
        if ( state )
            action->setChecked( true );
        else
            action->setChecked( false );
    }
        break;

    case NoType:
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
