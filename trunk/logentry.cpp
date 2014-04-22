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
#include "logentry.h"
#include "main.h"

/*
================
construct
================
*/
LogEntry::LogEntry( const QSqlRecord &record, const QString &table ) {
    this->setRecord( record );
    this->setTable( table );

    // perform updates
    this->connect( this, SIGNAL( changed()), &m, SLOT( update()));
}

/*
================
points
================
*/
int LogEntry::points() const {
    TaskEntry *taskPtr = m.taskForId( this->taskId());

    if ( taskPtr != NULL )
        return taskPtr->calculate( this->id());

    return 0;
}
