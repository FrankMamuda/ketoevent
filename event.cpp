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
#include "field.h"
#include "database.h"

//
// namespaces
//
using namespace EventTable;

/**
 * @brief Event::Event
 * @param parent
 */
Event::Event() : Table( "events" ) {
    this->addField( ID,      "id",                 QVariant::UInt,   "integer primary key", true, true );
    this->addField( API,     "api",                QVariant::Int,    "integer" );
    this->addField( Title,   "name",               QVariant::String, "text",       true );
    this->addField( Min,     "minMembers",         QVariant::Int,    "integer" );
    this->addField( Max,     "maxMembers",         QVariant::Int,    "integer" );
    this->addField( Start,   "startTime",          QVariant::String, "text" );
    this->addField( Finish,  "finishTime",         QVariant::String, "text" );
    this->addField( Final,   "finalTime",          QVariant::String, "text" );
    this->addField( Penalty, "penalty",            QVariant::Int,    "integer" );
    this->addField( Combo2,  "comboOfTwo",         QVariant::Int,    "integer" );
    this->addField( Combo3,  "comboOfThree",       QVariant::Int,    "integer" );
    this->addField( Combo4,  "comboOfFourAndMore", QVariant::Int,    "integer" );
    this->addField( Lock,    "lock",               QVariant::Int,    "integer" );
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
