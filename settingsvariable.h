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

#ifndef SETTINGSVARIABLE_H
#define SETTINGSVARIABLE_H

//
// includes
//
#include <QSpinBox>
#include <QCheckBox>
#include <QTimeEdit>
#include <QLineEdit>

//
// class:pSettingsCvar
//
class SettingsVariable : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString key READ key WRITE setKey )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_ENUMS( Types )
    Q_ENUMS( Class )

public:
    // currently supported types
    enum Types {
        NoType = -1,
        CheckBox,
        SpinBox,
        TimeEdit,
        LineEdit
    };
    // currently supported types
    enum Class {
        NoClass = -1,
        ConsoleVar,
        EventVar
    };
    QString key() const { return this->m_key; }
    Types type () const { return this->m_type; }
    Class varClass() const { return this->m_class; }

    // constructor
    SettingsVariable( const QString &key, SettingsVariable::Types bType, SettingsVariable::Class varClass = ConsoleVar );

    // set initial values from cvars
    void setState();

    // save value
    void save();

    // bind/unbind
    void unbind();
    void bind( QObject *objPtr, QObject *parentPtr );

public slots:
    void setKey( const QString &key ) { this->m_key = key; }
    void setType( Types bType ) { this->m_type = bType; }
    void setClass( Class varClass ) { this->m_class = varClass; }

    // checkBox
    void stateChanged( int state );

    // spinBox
    void integerValueChanged( int integer );

    // timeEdit
    void timeChanged( const QTime &time );

    // textChanged
    void textChanged( const QString &text );

private:
    Types m_type;
    Class m_class;
    QString m_key;
    QObject *objPtr;
};

#endif // SETTINGSVARIABLE_H