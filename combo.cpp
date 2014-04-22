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

//
// combo.cpp (main.cpp is too crowded)
//


//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
getFreeComboId
================
*/
int Main::getFreeComboId() {
    int highest = 0;
    foreach ( LogEntry *logPtr, this->logList ) {
        if ( logPtr->comboId() >= highest )
            highest = logPtr->comboId();
    }
    return highest + 1;
}

/*
================
removeOrphanedCombos
================
*/
void Main::removeOrphanedCombos() {
    // we need at least two matches to
    foreach ( LogEntry *logPtr0, this->logList ) {
        bool found = false;

         foreach ( LogEntry *logPtr1, this->logList ) {
             if ( logPtr0->comboId() == logPtr1->comboId()) {
                 found = true;
                 break;
             }
         }
         if ( !found )
             logPtr0->setComboId( -1 );
    }

#if 0
    // create query
    QSqlQuery query;

    // remove orphaned combos
    if ( !query.exec( "delete from combos where idString=0" ) || !query.exec( "delete from combos where teamId not in ( select id from teams )" ))
        m.error( StrSoftError + QString( "could not delete orphaned combos, reason: %1\n" ).arg( query.lastError().text()));


    // remove duplicates (should not happen)

    /*foreach ( ComboEntry *comboPtr, this->comboList ) {
        if ( comboPtr->teamId() == teamId && !QString::compare( comboPtr->idString(), == teamId )
            return logPtr;
    }*/
#endif
}
