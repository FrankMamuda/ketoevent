/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

/*
 * includes
 */
#include "event.h"
#include "database.h"

// singleton
Event *Event::i = nullptr;

//
// namespaces
//
using namespace EventTable;

/**
 * @brief Event::Event
 * @param parent
 */
Event::Event() : Table( "events" ) {
    PRIMARY_FIELD( ID );
    FIELD( API,     QMetaType::Int );
    FIELD( Title,   QMetaType::QString );
    FIELD( Min,     QMetaType::Int );
    FIELD( Max,     QMetaType::Int );
    FIELD( Start,   QMetaType::QString );
    FIELD( Finish,  QMetaType::QString );
    FIELD( Final,   QMetaType::QString );
    FIELD( Penalty, QMetaType::Int );
    FIELD( Combo2,  QMetaType::Int);
    FIELD( Combo3,  QMetaType::Int );
    FIELD( Combo4,  QMetaType::Int );
    FIELD( Lock,    QMetaType::Int );
}

/**
 * @brief Event::add
 * @param title
 */
Row Event::add( const QString &title, int minMembers, int maxMembers,
                 const QTime &start, const QTime &finish, const QTime &final,
                 int penalty, int two, int three, int fourPlus ) {
    return Table::add( QVariantList() <<
                Database_::null <<
                Version <<
                title <<
                minMembers <<
                maxMembers <<
                start.toString( Database_::TimeFormat ) <<
                finish.toString( Database_::TimeFormat ) <<
                final.toString( Database_::TimeFormat ) <<
                penalty <<
                two <<
                three <<
                fourPlus <<
                       0 );
}
