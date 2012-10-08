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

//
// NOTE: using a modified YPlatorm2 (r25) code
//

//
// includes
//
#include "sys_cvarfunc.h"
#include "sys_cvar.h"
#include "sys_common.h"

/*
============
construct
============
*/
pCvar::pCvar( const QString &name, const QString &string, Flags flags ) {
    // set the defaults
    this->flags = flags;
    this->setName( name );
    this->setString( string );
    this->setResetString( string );
}

/*
============
integer
============
*/
int pCvar::integer() const {
    bool valid;
    int y;

    // set integer if any
    y = this->string().toInt( &valid );

    // all ok, return integer value
    if ( valid )
        return y;

    return 0;
}

/*
============
time

  special func for ketoevent applet
============
*/
QTime pCvar::time() const {
    QStringList timeStr;
    int hour = 0, minute = 0, second = 0;

    timeStr = this->string().split( ":" );
    if ( timeStr.count() >= 1 )
        hour = timeStr.at( 0 ).toInt();
    if ( timeStr.count() >= 2 )
        minute = timeStr.at( 1 ).toInt();
    if ( timeStr.count() == 3 )
        second = timeStr.at( 3 ).toInt();

    return QTime( hour, minute, second );
}

/*
============
value
============
*/
float pCvar::value() const {
    bool valid;
    float y;

    // set integer if any
    y = this->string().toFloat( &valid );

    // all ok, return float value
    if ( valid )
        return y;

    return 0;
}

/*
============
set
============
*/
bool pCvar::set( const QString &string, AccessFlags access ) {
    if ( this->flags.testFlag( ReadOnly ) && !access.testFlag( Force ))
        com.error( StrSoftError + this->tr( " \"%1\" is read only\n" ).arg( this->name()));
    else {
        this->setString( string );
        emit valueChanged( this->name(), this->string());
    }
    return true;
}

/*
============
set
============
*/
bool pCvar::set( int integer, AccessFlags access ) {
    return this->set( QString( "%1" ).arg( integer ), access );
}

/*
============
set
============
*/
bool pCvar::set( double integer, AccessFlags access ) {
    return this->set( QString( "%1" ).arg( integer ), access );
}

/*
============
set
============
*/
bool pCvar::set( float value, AccessFlags access ) {
    return this->set( QString( "%1" ).arg( value ), access );
}

/*
============
reset
============
*/
void pCvar::reset() {
    this->setString( this->resetString());
}
