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

#ifndef SYS_CVAR_H
#define SYS_CVAR_H

//
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "sys_shared.h"
#include "sys_cvarfunc.h"
#include <QRegExpValidator>

//
// namespaces
//
namespace Cvar {
    class Sys_Cvar;
    static const QString DefaultConfigFile( "config/platform.xml" );
}

//
// class:Sys_Cvar
//
class Sys_Cvar : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Console variable subsystem" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )

public:
    pCvar *find( const QString &name ) const;
    void saveConfig( const QString &filename );
    void parseConfig( const QString &filename );
    pCvar *create( const QString &name, const QString &string, pCvar::Flags flags = pCvar::NoFlags );
    bool hasInitialized() const { return this->m_initialized; }

public slots:
    void clear();
    void init();
    void shutdown();
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }

private:
    QList<pCvar*> cvarList;
    bool validate( const QString &s ) const;
    QRegExpValidator *validator;
    bool m_initialized;
};

//
// externals
//
extern class Sys_Cvar cv;

#endif // SYS_CVAR_H
