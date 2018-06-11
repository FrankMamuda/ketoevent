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

//
// TODO: use string literals
//

//
// sort out ui element naming
// nameType vs typeName
// closeButton vs buttonClose
//


/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    // set variable defaults
    Variable::instance()->add( "reviewerName", "" );

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

    // clean up on exit
    qApp->connect( qApp, &QApplication::aboutToQuit, []() {
        XMLTools::instance()->write();

        if ( Database::instance() != nullptr )
            delete Database::instance();

        if ( EditorDialog::instance() != nullptr )
            delete EditorDialog::instance();

        // remove toolbars, team task widgets, mainwindow
    } );

    return a.exec();
}
