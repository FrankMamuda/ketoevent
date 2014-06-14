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
// reviewer.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>

/*
================
addReviewer
================
*/
void Main::addReviewer( const QString &name ) {
    QSqlQuery query;
    QString reviewerName;

    // copy title
    reviewerName = name;

    // set title if none
    if ( reviewerName.isEmpty())
        reviewerName = this->tr( "unnamed reviewer" );

    // check for duplicates
    foreach ( ReviewerEntry *rPtr, this->base.reviewerList ) {
        if ( !QString::compare( rPtr->name(), reviewerName ))
            continue;
    }

    // add new reviewer to database
    if ( !query.exec( QString( "insert into reviewers values ( null, '%1' )" ).arg( reviewerName )))
        this->error( StrSoftError + QString( "could not add reviewer, reason - \"%1\"\n" ).arg( query.lastError().text()));

    // get latest id
    query.exec( QString( "select * from reviewers where id=%1" ).arg( query.lastInsertId().toInt()));

    // get last sql entry and construct internal entry
    while ( query.next()) {
        this->base.reviewerList << new ReviewerEntry( query.record(), "reviewers" );
        break;
    }
}

/*
================
loadReviewers
================
*/
void Main::loadReviewers( bool import, int offset ) {
    QSqlQuery query;
    Q_UNUSED( import )
    Q_UNUSED( offset )

    // read all reviewer entries
    query.exec( "select * from reviewers" );

    // store entries in memory
    while ( query.next())
        this->base.reviewerList << new ReviewerEntry( query.record(), "reviewers" );
}

/*
================
reviewerForId
================
*/
ReviewerEntry *Main::reviewerForId( int id ) {
    foreach ( ReviewerEntry *rPtr, this->base.reviewerList ) {
        if ( rPtr->id() == id )
            return rPtr;
    }
    return NULL;
}
