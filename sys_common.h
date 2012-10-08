/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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

#ifndef SYS_COMMON_H
#define SYS_COMMON_H

//
// includes
//
#include "sys_shared.h"

//
// namespaces
//
namespace Common {
    class Sys_Common;
}

//
// class:Sys_Common
//
class Sys_Common : public QObject {
    Q_OBJECT
    Q_ENUMS( ErrorTypes )

public:
    // enums
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };
    void print( const QString &msg );
    void error( ErrorTypes type, const QString &msg );
    int minutesFromQTime( const QTime &time ) const;
};

//
// externals
//
extern class Sys_Common com;

#endif // SYS_COMMON_H
