/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "singleton.h"
#include <QDir>
#include <QMap>

/**
 * @brief The XML namespace
 */
namespace XML {
const static QString Variables( "configuration.xml" );
const static QString ConfigPath( QDir::homePath() + "/.ketoevent/" );
}

/**
 * @brief The XMLTools class
 */
class XMLTools : public QObject {
    Q_OBJECT

public:
    ~XMLTools() {}
    static XMLTools *instance() { return Singleton<XMLTools>::instance( XMLTools::createInstance ); }
    void write();
    void read();

private:
    XMLTools( QObject *parent = nullptr ) : QObject( parent ) {}
    static XMLTools *createInstance() { return new XMLTools(); }
};
