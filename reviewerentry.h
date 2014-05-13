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

#ifndef REVIEWERENTRY_H
#define REVIEWERENTRY_H

//
// includes
//
#include "databaseentry.h"

//
// class: ReviewerEntry
//
class ReviewerEntry : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_CLASSINFO( "description", "Reviewer SQL Entry" )

public:
    explicit ReviewerEntry( const QSqlRecord &record, const QString &table );
    QString name() const { return this->record().value( "name" ).toString(); }

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
};

#endif // REVIEWERENTRY_H
