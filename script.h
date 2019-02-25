/*
 * Copyright (C) 2019 Factory #12
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
#include <QJSEngine>

/**
 * @brief The EventTable namespace
 */
namespace Script_ {
const static unsigned int API = 0;
}

/**
 * @brief The Script class
 */
class Script final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Script )

public:
    /**
     * @brief instance
     * @return
     */
    static Script *instance() { static Script *instance = new Script(); return instance; }
    virtual ~Script() override {}
    QJSValue call( const QString &func, const QJSValueList &args = QJSValueList()) const;

public slots:
    bool evaluate( const QString &script );

private:
    explicit Script();
    QJSEngine engine;
};
