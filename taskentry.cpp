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
// includes
//
#include "taskentry.h"
#include "logentry.h"
#include "main.h"

/*
================
construct
================
*/
TaskEntry::TaskEntry( const QSqlRecord &record, const QString &table ) {
    // bind to sql
    this->setRecord( record );
    this->setTable( table );

    // points failsafe
    if ( this->points() < 0 )
        this->setPoints();

    // maximum multiplier failsafe
    if ( this->multi() < 0 )
        this->setMulti();

    // type failsafe
    switch ( this->type()) {
    case Check:
    case Multi:
        break;

    default:
        this->setType();
    }

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/*
================
calculate
================
*/
int TaskEntry::calculate( int logId ) const {
    int value = 0;

    // get log parent
    LogEntry *logPtr = m.logForId( logId );
    if ( logPtr == NULL )
        return 0;

    // get initial points
    switch ( this->type()) {
    case Check:
        if ( logPtr->value())
            value += this->points();
        break;

    case Multi:
        if ( logPtr->value() > this->multi())
            value += this->points() * this->multi();
        else
            value += this->points() * logPtr->value();
        break;

    default:
        // error
        return 0;
    }

    // get combo points
    // TODO/FIXME: MUST COUNT COMBO POINTS ELSEWHERE
    // ALSO CHECK FOR TOOLTIPS (correct points)
  //  foreach ( ComboEntry *cPtr, )
#if 0
    switch ( logPtr->combo()) {
    case LogEntry::Single:
        value += m.currentEvent()->singleCombo();
        break;

    case LogEntry::Double:
        value += m.currentEvent()->doubleCombo();
        break;

    case LogEntry::Triple:
        value += m.currentEvent()->tripleCombo();
        break;

    default:
    case LogEntry::NoCombo:
        break;
    }
#endif

    // we're done
    return value;
}
