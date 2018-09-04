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
#include "mainwindow.h"
#include <QApplication>
#include "database.h"
#include "event.h"
#include "task.h"
#include "team.h"
#include "log.h"
#include "editordialog.h"
#include "xmltools.h"
#include "variable.h"
#include "console.h"
#include "main.h"
#include "item.h"
#include "rankingsmodel.h"
#include "widget.h"

//
// GENERAL TODO LIST
//
//   - implement working combos
//   - implement imports (at least pure stats)
//

// default message handler
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler( 0 );

/**
 * @brief messageFilter
 * @param type
 * @param context
 * @param msg
 */
void messageFilter( QtMsgType type, const QMessageLogContext &context, const QString &msg ) {
    ( *QT_DEFAULT_MESSAGE_HANDLER )( type, context, msg );

    if ( type == QtFatalMsg ) {
        QApplication::quit();
        exit( 0 );
    }

    if ( Main::Console != nullptr )
        qobject_cast<Console*>( Main::Console )->print( msg );
}

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    // register metatypes
    qRegisterMetaType<Item::Actions>();
    qRegisterMetaType<Item::Types>();
    qRegisterMetaType<Event::Fields>();
    qRegisterMetaType<Log::Fields>();
    qRegisterMetaType<Log::Roles>();
    qRegisterMetaType<Id>();
    qRegisterMetaType<RankingsModel::Columns>();
    qRegisterMetaType<Table::Roles>();
    qRegisterMetaType<Task::Fields>();
    qRegisterMetaType<Task::Types>();
    qRegisterMetaType<Task::Styles>();
    qRegisterMetaType<Team::Fields>();
    //qRegisterMetaType<Var::Flags>();
    qRegisterMetaType<Widget::Types>();

    // set console output pattern
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );

    // log to file in non-qtcreator environment
    qInstallMessageHandler( messageFilter );

    // set variable defaults
    Variable::instance()->add( "reviewerName", "" );
    Variable::instance()->add( "eventId", -1 );
    Variable::instance()->add( "teamId", -1 );
    Variable::instance()->add( "rankingsCurrent", true );
    Variable::instance()->add( "sortByType", true );
    Variable::instance()->add( "system/consoleHistory", "" );

    // read configuration
    XMLTools::instance()->read();

    // initialize database and its tables
    Database::instance();
    Database::instance()->add( Event::instance());
    Database::instance()->add( Task::instance());
    Database::instance()->add( Team::instance());
    Database::instance()->add( Log::instance());

    // show main window
    MainWindow::instance()->show();

    // initialize console
    Main::Console = Console::instance();

    // clean up on exit
    qApp->connect( qApp, &QApplication::aboutToQuit, []() {
        delete Console::instance();
        Main::Console = nullptr;

        XMLTools::instance()->write();
        GarbageMan::instance()->clear();

        delete GarbageMan::instance();

        if ( Database::instance() != nullptr )
            delete Database::instance();

        delete Variable::instance();
    } );

    return a.exec();
}
