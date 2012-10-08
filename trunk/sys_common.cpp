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

//
// includes
//
#include "sys_common.h"
#include "app_main.h"

//
// classes
//
class Sys_Common com;

/*
============
print
============
*/
void Sys_Common::print( const QString &msg ) {
    // for debugging
    if ( msg.endsWith( "\n" ))
        qDebug() << msg.left( msg.length()-1 );
    else
        qDebug() << msg;
}

/*
============
error
============
*/
void Sys_Common::error( ErrorTypes type, const QString &msg ) {
    if ( type == FatalError ) {
        this->print( this->tr( "FATAL ERROR: %1" ).arg( msg ));
        m.shutdown();
    } else
        this->print( this->tr( "ERROR: %1" ).arg( msg ));
}

/*
============
minutesFromQTime
============
*/
int Sys_Common::minutesFromQTime( const QTime &time ) const {
    return time.hour() * 60 + time.minute();
}
