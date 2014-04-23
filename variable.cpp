/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
// variable.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"

/*
================
addCvar
================
*/
void Main::addCvar( ConsoleVariable *varPtr ) {
    // avoid duplicates
    if ( this->cvar( varPtr->key())) {
        delete varPtr;
        return;
    }

    this->cvarList << varPtr;
}

/*
================
addSvar
================
*/
void Main::addSvar( const QString &key, SettingsVariable::Types type, SettingsVariable::Class varClass ) {
    this->svarList << new SettingsVariable( key, type, varClass );
}

/*
================
cvar
================
*/
ConsoleVariable *Main::cvar( const QString &key ) {
    foreach ( ConsoleVariable *varPtr, this->cvarList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return this->defaultCvar;
}

/*
================
svar
================
*/
SettingsVariable *Main::svar( const QString &key ) {
    foreach ( SettingsVariable *varPtr, this->svarList ) {
        if ( !QString::compare( varPtr->key(), key ))
            return varPtr;
    }
    return this->defaultSvar;
}
