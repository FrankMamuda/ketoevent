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

#include "app_tasklog.h"
#include "sys_shared.h"
#include "app_main.h"

//
// cvars
//
extern pCvar *k_singleComboPoints;
extern pCvar *k_doubleComboPoints;
extern pCvar *k_tripleComboPoints;

/*
================
construct
================
*/
App_TaskLog::App_TaskLog( int value, Combos combo, App_TaskEntry *taskPtr ) {
    // initialize values
    this->setValue( value );
    this->setCombo( combo );
    this->setTask( taskPtr );
}

/*
================
comboPoints
================
*/
int App_TaskLog::comboPoints() const {
    switch ( this->combo()) {
    case Single:
        return k_singleComboPoints->integer();

    case Double:
        return k_doubleComboPoints->integer();

    case Triple:
        return k_tripleComboPoints->integer();

    default:
    case NoCombo:
        return 0;
    }
}

/*
================
generateXMLTag
================
*/
QString App_TaskLog::generateXMLTag() {
    int value = 0;

    // get check or multi value
    if ( this->task()->type() == App_TaskEntry::Check )
        value = this->check();
    else if ( this->task()->type() == App_TaskEntry::Multi || this->task()->type() == App_TaskEntry::Special )
        value = this->value();

    // no points? don't bother
    if ( value == 0 )
        return QString::null;

    // at this point we don't actually care about current points or type
    // everything is to be recalculated on load
    if ( this->task()->type() == App_TaskEntry::Check || this->task()->type() == App_TaskEntry::Multi )
        return QString( "  <log hash=\"%1\" value=\"%2\" combo=\"%3\"/>\n" )
                .arg( this->task()->hash())
                .arg( value )
                .arg( static_cast<int>( this->combo()));
    else
        return QString( "  <log hash=\"%1\" value=\"%2\"/>\n" )
                .arg( this->task()->hash())
                .arg( value );
}
