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
// includes
//
#include "databaseentry.h"
#include <QSqlQuery>
#include <QSqlError>
#include "main.h"

/*
================
setValue
================
*/
void DatabaseEntry::setValue( const QString &name, const QVariant &value ) {
    QSqlQuery query;
    QVariant update;

    // copy local value
    update = value;

    // make sure we don't perform useless updates
    if ( this->record().value( name ) == update )
        return;

    // update counters
    emit this->changed();

    // store local value
    this->m_record.setValue( name, update );

    // check for strings (should be wrapped in quotes)
    if ( value.type() == QVariant::String )
        update.setValue( QString( "'%1'" ).arg( value.toString()));

    // update database value
    if ( !this->table().isNull()) {
        if ( !query.exec( QString( "update %1 set %2 = %3 where id=%4" ).arg( this->table()).arg( name ).arg( update.toString()).arg( this->record().value( "id" ).toInt())))
            m.error( StrSoftError + this->tr( "could not store value, reason - \"%1\"\n" ).arg( query.lastError().text()));
    }
}
