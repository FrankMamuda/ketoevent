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
// FIXME: for some reason there's always an empty (nameless) variable in map
//

//
// includes
//
#include "variable.h"
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDebug>
#include <QLineEdit>
#include <QTimeEdit>
#include <QAction>

/**
 * @brief Variable::Variable
 */
Variable::Variable() : signalMapper( new QSignalMapper( this )) {
    this->connect( this, &Variable::valueChanged, [ this ]( const QString &key ) { this->setBoundValue( key, false ); } );
    this->connect( this->signalMapper, static_cast< void( QSignalMapper::* )( const QString & )>( &QSignalMapper::mapped ), [ this ]( const QString &key ) { this->setBoundValue( key, true ); } );
}

/**
 * @brief Variable::~Variable
 */
Variable::~Variable() {
    this->disconnect( this, SIGNAL( valueChanged( QString )));
    this->disconnect( this->signalMapper, SIGNAL( mapped( QString )));
    delete this->signalMapper;
}

/**
 * @brief Variable::bind
 * @param key
 * @param receiver
 * @param method
 */
void Variable::bind( const QString &key, const QObject *receiver, const char *method ) {
   QPair<QObject*, int> slot;
   int code;

    if ( key.isEmpty())
        return;

    // check if method is a slot
    code = (( static_cast<int>( *method ) - '0' ) & 0x3 );
    if ( code != 1 )
        return;

    // get method name
    ++method;

    // create an object/method pair
    slot.first = const_cast<QObject*>( receiver );
    slot.second = receiver->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( qPrintable( method )));

    // add pair to slotList
    this->slotList[key] = slot;
}

/**
 * @brief Variable::bind
 * @param key
 * @param widget
 */
QString Variable::bind( const QString &key, QObject *object ) {
    this->boundVariables[key] = object;
    this->setBoundValue( key, false );

    // determine widget type
    if ( !QString::compare( object->metaObject()->className(), "QCheckBox" )) {
        QCheckBox *checkBox;
        checkBox = qobject_cast<QCheckBox*>( object );

        // connect for further updates
        if ( checkBox != nullptr ) {
            this->connect( checkBox, SIGNAL( stateChanged( int )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( checkBox, key );
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QAction" )) {
        QAction *action;
        action = qobject_cast<QAction*>( object );

        // connect for further updates
        if ( action != nullptr ) {
            this->connect( action, SIGNAL( toggled( bool )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( action, key );
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QLineEdit" )) {
        QLineEdit *lineEdit;
        lineEdit = qobject_cast<QLineEdit*>( object );

        // connect for further updates
        if ( lineEdit != nullptr ) {
            this->connect( lineEdit, SIGNAL( textChanged( QString )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( lineEdit, key );
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QTimeEdit" )) {
        QTimeEdit *timeEdit;
        timeEdit = qobject_cast<QTimeEdit*>( object );

        // connect for further updates
        if ( timeEdit != nullptr ) {
            this->connect( timeEdit, SIGNAL( timeChanged( QTime )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( timeEdit, key );
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QSpinBox" )) {
        QSpinBox *spinBox;
        spinBox = qobject_cast<QSpinBox*>( object );

        // connect for further updates
        if ( spinBox != nullptr ) {
            this->connect( spinBox, SIGNAL( valueChanged( int )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( spinBox, key );
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QComboBox" )) {
        QComboBox *comboBox;
        comboBox = qobject_cast<QComboBox*>( object );

        // connect for further updates
        if ( comboBox != nullptr ) {
            this->connect( comboBox, SIGNAL( currentIndexChanged( int )), this->signalMapper, SLOT( map()));
            this->signalMapper->setMapping( comboBox, key );
        }
    } else {
        qCWarning( Variable_::debug, this->tr( "unsupported container \"%1\"" ).arg( object->metaObject()->className()).toUtf8().constData());
    }

    return key;
}

/**
 * @brief Variable::unbind
 * @param key
 */
void Variable::unbind( const QString &key ) {
    if ( this->boundVariables.contains( key )) {
        QObject *object;

        object = this->boundVariables[key];
        this->signalMapper->removeMappings( object );
        this->boundVariables.remove( key );
    }
}

/**
 * @brief Variable::setBoundValue
 * @param key
 * @param internal
 */
void Variable::setBoundValue( const QString &key, bool internal ) {
    QObject *object;

    if ( !this->boundVariables.contains( key ))
        return;

    // get widget and block it's signals
    object = this->boundVariables[key];
    object->blockSignals( true );

    // determine widget type
    if ( !QString::compare( object->metaObject()->className(), "QCheckBox" )) {
        QCheckBox *checkBox;

        checkBox = qobject_cast<QCheckBox*>( object );

        if ( checkBox != nullptr ) {
            if ( internal )
                this->setValue<bool>( key, checkBox->isChecked(), true );
            else
                checkBox->setChecked( this->isEnabled( key ));
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QAction" )) {
        QAction *action;

        action = qobject_cast<QAction*>( object );

        if ( action != nullptr ) {
            if ( internal )
                this->setValue<bool>( key, action->isChecked(), true );
            else
                action->setChecked( this->isEnabled( key ));
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QTimeEdit" )) {
        QTimeEdit *timeEdit;

        timeEdit = qobject_cast<QTimeEdit*>( object );

        if ( timeEdit != nullptr ) {
            QTime time;

            if ( internal )
                this->setValue<QTime>( key, timeEdit->time(), true );
            else
                timeEdit->setTime( this->value<QTime>( key ));
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QLineEdit" )) {
        QLineEdit *lineEdit;

        lineEdit = qobject_cast<QLineEdit*>( object );

        if ( lineEdit != nullptr ) {
            if ( internal )
                this->setValue<QString>( key, lineEdit->text(), true );
            else
                lineEdit->setText( this->string( key ));
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QSpinBox" )) {
        QSpinBox *spinBox;

        spinBox = qobject_cast<QSpinBox*>( object );

        if ( spinBox != nullptr ) {
            if ( internal )
                this->setValue<int>( key, spinBox->value(), true );
            else
                spinBox->setValue( this->integer( key ));
        }
    } else if ( !QString::compare( object->metaObject()->className(), "QComboBox" )) {
        QComboBox *comboBox;

        comboBox = qobject_cast<QComboBox*>( object );

        if ( comboBox != nullptr ) {
            if ( comboBox->currentIndex() != -1 ) {
                if ( internal )
                    this->setValue( key, comboBox->currentData(), true );
                else {
                    int y;
                    for ( y = 0; y < comboBox->count(); y++ ) {
                        if ( comboBox->itemData( y ) == this->value<QVariant>( key )) {
                            comboBox->setCurrentIndex( y );
                            break;
                        }
                    }
                }
            } else {
                qCWarning( Variable_::debug, this->tr( "void comboBox for variable \"%1\"" ).arg( key ).toUtf8().constData());
            }
        }
    } else {
        qCWarning( Variable_::debug, this->tr( "unsupported container \"%1\"" ).arg( object->metaObject()->className()).toUtf8().constData());
    }

    // force update
    if ( internal )
        this->updateConnections( key, this->value<QVariant>( key ));

    // unblock signals
    object->blockSignals( false );
}
