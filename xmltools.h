/*
 * Copyright (C) 2017-2018 Factory #12
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

#pragma once

//
// includes
//
#include "main.h"

#include <QDir>
#include <QLoggingCategory>
#include <QMap>

/**
 * @brief The XML namespace
 */
namespace XMLTools_ {
#ifdef Q_CC_MSVC
static constexpr const *ConfigFile( "configuration.xml" );
#else
static constexpr const char __attribute__((unused)) *ConfigFile( "configuration.xml" );
#endif
const static QLoggingCategory Debug( "xml" );
}

/**
 * @brief The XMLTools class
 */
class XMLTools final : public QObject {
    Q_DISABLE_COPY( XMLTools )
    Q_OBJECT

public:
    virtual ~XMLTools() = default;
    static XMLTools *instance() { static XMLTools *instance( new XMLTools()); return instance; }
    void write();
    void read();

private:
    explicit XMLTools( QObject *parent = nullptr ) : QObject( parent ) { this->setObjectName( "XMLTools" ); GarbageMan::instance()->add( this ); }
};
