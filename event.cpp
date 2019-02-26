/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include "event.h"
#include "field.h"
#include "database.h"
#include "variable.h"

//
// namespaces
//
using namespace EventTable;

/**
 * @brief Event::Event
 * @param parent
 */
Event::Event() : Table( EventTable::Name ) {
    this->addField( ID,      "id",                 QVariant::UInt,   "integer primary key", true, true );
    // NOTE: are we even checking api?
    this->addField( API,     "api",                QVariant::Int,    "integer" );
    /* renameme */ this->addField( Title,   "name",               QVariant::String, "varchar( 64 )",       true );
    this->addField( Min,     "minMembers",         QVariant::Int,    "integer" );
    this->addField( Max,     "maxMembers",         QVariant::Int,    "integer" );
    /* removeme */ this->addField( Start,   "startTime",          QVariant::String, "varchar( 5 )" );
    /* removeme */ this->addField( Finish,  "finishTime",         QVariant::String, "varchar( 5 )" );
    /* removeme */ this->addField( Final,   "finalTime",          QVariant::String, "varchar( 5 )" );
    /* removeme */ this->addField( Penalty, "penalty",            QVariant::Int,    "integer" );
    /* removeme */ this->addField( Combo2,  "comboOfTwo",         QVariant::Int,    "integer" );
    /* removeme */ this->addField( Combo3,  "comboOfThree",       QVariant::Int,    "integer" );
    /* removeme */ this->addField( Combo4,  "comboOfFourAndMore", QVariant::Int,    "integer" );
    /* removeme */ this->addField( Lock,    "lock",               QVariant::Int,    "integer" );
    this->addField( Script,  "script",             QVariant::String, "text" );
    /* addme this->addField( Parms, "parameters", QVariant::String, "text" );
      this is supposed to hold values to be passed to the scripts
    */

    // TODO: we must break API
    // for now only a script field is added
    // in upcoming builds fields such as times, penalties, combos etc. will be removed
    //   to be dynamically parsed from an event script
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
                       finish.toString (Database_::TimeFormat ) <<
                       final.toString( Database_::TimeFormat ) <<
                       penalty <<
                       two <<
                       three <<
                       fourPlus <<
                       0 );
}

/**
 * @brief Event::script
 * @param row
 * @return
 */
QString Event::script( const Row &row ) const {
#ifdef QT_DEBUG
    // TODO: must use script API

    // read script file
    auto loadScript = []( const QString &filename ) {
        QFile script( filename );
        if ( script.open( QIODevice::ReadOnly )) {
            const QString buffer( script.readAll());
            script.close();
            return buffer;
        }
        return QString();
    };

    QString script( loadScript( QFileInfo( Variable::instance()->string( "databasePath" )).absolutePath() + "/script.js" ));
    if ( script.isEmpty())
        loadScript( ":/scripts/default.js" );

    if ( !script.isEmpty())
        return script;

#endif
    return this->value( row, Script ).toString();
}

