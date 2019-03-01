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
    // NOTE: are we even checking api?
    this->addField( ID,      "id",      QVariant::UInt,   "integer primary key", true, true );
    this->addField( API,     "api",     QVariant::Int,    "integer"                         );
    this->addField( Title,   "title",   QVariant::String, "text",                true       );
    this->addField( Script,  "script",  QVariant::String, "text"                            );
    this->addField( Options, "options", QVariant::String, "text"                            );
}

/**
 * @brief Event::add
 * @param title
 */
Row Event::add( const QString &title, const QString &script, const QString &options ) {
    return Table::add( QVariantList() <<
                       Database_::null <<
                       Version <<
                       title <<
                       script <<
                       options );
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
        script = loadScript( ":/scripts/default.js" );

    if ( !script.isEmpty())
        return script;

#endif
    return this->value( row, Script ).toString();
}

