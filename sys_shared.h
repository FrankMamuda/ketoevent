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

#ifndef SYSSHARED_H
#define SYSSHARED_H

//
// includes
//
#include <QtGlobal>
#include <QtCore>
#include <QtXml>

// filesystem
typedef int fileHandle_t;
typedef unsigned char byte;

// message macro
#if ( _MSC_VER >= 1500 && _MSC_VER < 1600 )
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __FUNCTION__ )
#else
#define ClassFunc QString( "%1::%2: " ).arg( this->metaObject()->className()).arg( __func__ )
#endif

// error message macros
#define StrMsg ClassFunc
#define StrFatalError Sys_Common::FatalError, ClassFunc
#define StrSoftError Sys_Common::SoftError, ClassFunc
#define StrWarn QObject::trUtf8( "WARNING:" ) + ClassFunc

#endif
