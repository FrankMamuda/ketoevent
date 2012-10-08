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

#ifndef SYS_CVARFUNC_H
#define SYS_CVARFUNC_H

//
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "sys_shared.h"

//
// class:pCvar
//
class pCvar : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Console variable" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString string READ string WRITE setString )
    Q_PROPERTY( QString reset READ resetString WRITE setResetString )
    Q_FLAGS( Flags Flag )
    Q_FLAGS( AccessFlags AccessFlag )

public:
    // cvar flags
    enum Flag {
        NoFlags     = 0x0,
        Archive     = 0x1,
        Unused0     = 0x2,
        ReadOnly    = 0x4
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    // access flags
    enum AccessFlag {
        NoAccessFlags   = 0x0,
        Force           = 0x1
    };
    Q_DECLARE_FLAGS( AccessFlags, AccessFlag )
    Flags flags;
    
    // constructors/destructors
    pCvar( const QString &name, const QString &string, Flags flags = NoFlags );
    pCvar() {}

    // property getters
    QString name() const { return this->m_name; }
    QString string() const { return this->m_string; }
    QString resetString() const { return this->m_reset; }

    // other funcs
    int     integer() const;
    QTime   time() const;
    float   value() const;
    bool    set( const QString &string, AccessFlags = NoAccessFlags );
    bool    set( int, AccessFlags = NoAccessFlags );
    bool    set( double, AccessFlags = NoAccessFlags );
    bool    set( float, AccessFlags = NoAccessFlags );
    void    reset();

public slots:
    // property setters
    void setName( const QString &cvarName ) { this->m_name = cvarName; }
    void setString( const QString &string ) { this->m_string = string; }
    void setResetString( const QString &string ) { this->m_reset = string; }

signals:
    void valueChanged( const QString &cvar, const QString &stringValue );

private:
    // properties
    QString m_string;
    QString m_name;
    QString m_reset;
};

#endif // SYS_CVARFUNC_H
