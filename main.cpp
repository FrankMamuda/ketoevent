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

//
// includes
//
#include "main.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "console.h"
#include "cmd.h"
#include "xmltools.h"

//
// classes
//
//class Main m;

/*
TODO:
- teamListModel in comboBoxes
- proper settings variables
- remove orphaned logs on recalculation
  also check for duplicates in team->logList

FUTURE:
- use listView instead of listWidget to reduce memory footprint
- output verbocity levels per subsystem
- use homedir for mac (if applicable)
- combos for stressTest
- richText styling? html?
- unify task and team dialogs (create superclass - less dup code)

NOTES:
- currently, there is no need for an API change
- sort teams alphabetically?

MODERNIZE:
- lambdas, overrides, singletons
- threading (database, rankings)
- qDebug() string routing
- database code rewrite (singleton)
*/

/**
 * @brief Main::initialise
 * @param parent
 */
bool Main::initialise( QObject *parent ) {
    // announce
    Common::print( StrMsg + this->tr( "initialising system\n" ), Common::System );

    // init cvars
    Variable::instance()->add( "databasePath", this->path );
    Variable::instance()->add( "backup/perform", true );
    Variable::instance()->add( "backup/changes", 25 );
    Variable::instance()->add( "misc/sortTasks", true );
    Variable::instance()->add( "misc/hilightLogged", false );
    Variable::instance()->add( "currentEvent", -1 );
    Variable::instance()->add( "reviewerName", "" );
    Variable::instance()->add( "system/consoleHistory", "" );
    Variable::instance()->add( "rankings/current", true );
    //Variable::instance()->add( "filesystem/last", "" );

    // create settings variables
    SettingsVariable::add( "startTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    SettingsVariable::add( "finishTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    SettingsVariable::add( "finalTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    SettingsVariable::add( "penalty", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "comboOfTwo", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "comboOfThree", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "comboOfFourAndMore", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "minMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "maxMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    SettingsVariable::add( "backup/changes", SettingsVariable::SpinBox, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "backup/perform", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "misc/sortTasks", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "misc/hilightLogged", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "name", SettingsVariable::LineEdit, SettingsVariable::EventVar );
    SettingsVariable::add( "databasePath", SettingsVariable::LineEdit, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "reviewerName", SettingsVariable::LineEdit, SettingsVariable::ConsoleVar );
    SettingsVariable::add( "rankings/current", SettingsVariable::Action, SettingsVariable::ConsoleVar );

    // load vars
    XMLTools::instance()->read();

    // set parent
    this->setParent( parent );

    // init counters
    this->changesCounter = 0;
    this->setInitialised( false );

    // TODO: move path to Database class
    Database::makePath( Variable::instance()->string( "databasePath" ));
    Variable::instance()->setValue( "databasePath", this->path );

    // load database entries
    if ( !Database::load())
        return false;

    // init command subsystem
#ifdef APPLET_DEBUG
    this->console->loadHistory();
#endif

    // we're up
    this->setInitialised();

    // success
    return true;
}

/**
 * @brief Main::shutdown perform garbage collection
 * @param ignoreDatabase
 */
void Main::shutdown( bool ignoreDatabase ) {    
    // announce
    Common::print( StrMsg + this->tr( "performing shutdown\n" ), Common::System );

    if ( this->isInitialised()) {
        // clear parent
        this->setParent( nullptr );

        // save settings
#ifdef APPLET_DEBUG
        this->console->saveHisotry();
#endif
        // write ot configuration
        XMLTools::instance()->write();

        // clear entries
        this->clearEvent();

        // ckear settings vars
        foreach ( SettingsVariable *varPtr, this->svarList )
            delete varPtr;
        this->svarList.clear();

        // close console
#ifdef APPLET_DEBUG
        this->console->close();
#endif

        // close database
        if ( !ignoreDatabase )
            Database::unload();

        // reset initialisation state
        this->setInitialised( false );

        // close applet
        QApplication::quit();
    } else {
        Database::unload();
        exit( 0 );
    }
}

/**
 * @brief Main::update
 */
void Main::update() {
    this->changesCounter++;

    if ( this->changesCounter == Variable::instance()->integer( "backup/changes" ) && Variable::instance()->isEnabled( "backup/perform" )) {
        Database::writeBackup();
        this->changesCounter = 0;
    }
}

/**
 * @brief Main::clearEvent perform garbage collection
 */
void Main::clearEvent() {
    // get main screen
    MainWindow *gui = qobject_cast<MainWindow*>( this->parent());
    if ( gui == nullptr )
        return;

    // clear task widgets on main screen
    gui->clearTasks();

    // clear teams (logs should be cleaned automatically on destruct)
    foreach ( Team *teamPtr, Main::instance()->teamList ) {
        Main::instance()->teamList.removeOne( teamPtr );
        delete teamPtr;
    }
    Main::instance()->teamList.clear();

    // clear events
    foreach ( Event *eventPtr, Main::instance()->eventList ) {
        Main::instance()->eventList.removeOne( eventPtr );
        delete eventPtr;
    }
    Main::instance()->eventList.clear();

    // clear tasks
    foreach ( Task *taskPtr, Main::instance()->taskList ) {
        Main::instance()->taskList.removeOne( taskPtr );
        delete taskPtr;
    }
    Main::instance()->taskList.clear();
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication app( argc, argv );

    // init app
    QCoreApplication::setOrganizationName( "factory12" );
    QCoreApplication::setApplicationName( "ketoevent" );

    // set debug level
#ifdef APPLET_DEBUG
    Main::instance()->setDebugLevel( Common::System );
#else
    Main::instance()->setDebugLevel( Common::NoDebug );
#endif

    // i18n
    QTranslator translator;
    QString locale;
#ifndef FORCE_LATVIAN
    locale = QLocale::system().name();
#else
    locale = "lv_LV";
#endif
    translator.load( QString( ":/i18n/ketoevent_%1" ).arg( locale ));
    app.installTranslator( &translator );

    // init console (for debugging)
#ifdef APPLET_DEBUG
    Console::init();
#endif

    // init main window
    MainWindow gui;
    gui.show();

    // initialise application
    if ( Main::instance()->initialise( qobject_cast<QObject*>( &gui ))) {

        // add teams
        gui.initialise();
    }

    // exec app
    return app.exec();
}
