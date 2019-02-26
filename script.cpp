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

//
// includes
//
#include "event.h"
#include "log.h"
#include "mainwindow.h"
#include "script.h"
#include "task.h"
#include "team.h"
#include <QDebug>

/**
 * @brief Script::Script
 */
Script::Script() {
    // add database related tables to the engine
    this->engine.globalObject().setProperty( "Team",  this->engine.newQObject( Team::instance()));
    this->engine.globalObject().setProperty( "Task",  this->engine.newQObject( Task::instance()));
    this->engine.globalObject().setProperty( "Event", this->engine.newQObject( Event::instance()));
    this->engine.globalObject().setProperty( "Log",   this->engine.newQObject( Log::instance()));
    this->engine.globalObject().setProperty( "Main",  this->engine.newQObject( MainWindow::instance()));
    this->engine.globalObject().setProperty( "JS",    this->engine.newQObject( this ));

    // enable console logging
    this->engine.installExtensions( QJSEngine::ConsoleExtension );
}

/**
 * @brief Script::call
 * @param func
 * @param args
 * @return
 */
QJSValue Script::call(const QString &func, const QJSValueList &args) const {
    const QJSValue globalObject( this->engine.globalObject());

    // check if function object exists
    if ( !globalObject.hasProperty( func )) {
        qDebug() << "no such function" << func;
        return QJSValue();
    }

    // check if it really callable
    QJSValue function( this->engine.globalObject().property( func ));
    if ( !function.isCallable()) {
        qDebug() << "function is not callable" << func;
        return QJSValue();
    }

    // call function
    return function.call( args );
}

/**
 * @brief Script::timeFromString
 * @param timeString
 * @return
 */
QTime Script::timeFromString( const QString &timeString ) const {
    return QTime::fromString( timeString, Database_::TimeFormat );
}

/**
 * @brief Script::currentTime
 * @return
 */
QTime Script::currentTime() const {
    return QTime::currentTime();
}

/**
 * @brief Script::evaluate
 * @param script
 * @return
 */
bool Script::evaluate( const QString &script ) {
    // evaluate script
    if ( this->engine.evaluate( script ).isError()) {
        qWarning() << "script evaluation failed";
        return false;
    }

    return true;
}
