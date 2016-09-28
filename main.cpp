/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "gui_main.h"
#include <QApplication>
#include <QTranslator>
#include "gui_console.h"
#include "cmd.h"

//
// classes
//
class Main m;

/*
TODO:
FUTURE:
- use listView instead of listWidget to reduce memory footprint
- output verbocity levels per subsystem
- use homedir for mac (if applicable)
- combos for stressTest
- richText styling? html?
- unify task and team dialogs (create superclass - less dup code)
- currently, there is no need for an API change
- sort teams alphabetically?
*/

/**
 * @brief Main::initialise
 * @param parent
 */
bool Main::initialise( QObject *parent ) {
    // announce
    Common::print( StrMsg + this->tr( "initialising system\n" ), Common::System );

    // set parent
    this->setParent( parent );

    // init counters
    this->changesCounter = 0;
    this->setInitialised( false );

    // initialise settings
    this->settings = new QSettings( "avoti", "ketoevent" );
    this->settings->setDefaultFormat( QSettings::NativeFormat );

    // initialise this first
    Variable::add( "databasePath", this->settings, this->path );

    // TODO: move path to Database class
    Database::makePath( Variable::string( "databasePath" ));
    Variable::setValue( "databasePath", this->path );

    // init cvars
    Variable::add( "backup/perform", this->settings, true );
    Variable::add( "backup/changes", this->settings, 25 );
    Variable::add( "misc/sortTasks", this->settings, true );
    Variable::add( "misc/hilightLogged", this->settings, false );
    Variable::add( "currentEvent", this->settings, -1 );
    Variable::add( "reviewerName", this->settings, "" );
    Variable::add( "system/consoleHistory", this->settings, "" );
    Variable::add( "rankings/current", this->settings, true );
    //Variable::add( "filesystem/last", this->settings, "" );

    // load database entries
    if ( !Database::load())
        return false;

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

    // init command subsystem
#ifdef APPLET_DEBUG
    cmd.init();
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
        this->setParent( NULL );

        // save settings
#ifdef APPLET_DEBUG
        this->console->saveHisotry();
#endif
        if ( this->settings != NULL ) {
            this->settings->sync();
            delete this->settings;
        }

        // clear entries
        this->clearEvent();

        // clear console vars
        foreach ( Variable *varPtr, this->cvarList )
            delete varPtr;
        this->cvarList.clear();

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

    if ( this->changesCounter == Variable::value( "backup/changes" ) && Variable::isEnabled( "backup/perform" )) {
        Database::writeBackup();
        this->changesCounter = 0;
    }
}

/**
 * @brief Main::clearEvent perform garbage collection
 */
void Main::clearEvent() {
    // get main screen
    Gui_Main *gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui == NULL )
        return;

    // clear task widgets on main screen
    gui->clearTasks();

    // clear teams (logs should be cleaned automatically on destruct)
    foreach ( Team *teamPtr, m.teamList ) {
        m.teamList.removeOne( teamPtr );
        delete teamPtr;
    }
    m.teamList.clear();

    // clear events
    foreach ( Event *eventPtr, m.eventList ) {
        m.eventList.removeOne( eventPtr );
        delete eventPtr;
    }
    m.eventList.clear();

    // clear tasks
    foreach ( Task *taskPtr, m.taskList ) {
        m.taskList.removeOne( taskPtr );
        delete taskPtr;
    }
    m.taskList.clear();
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
    QCoreApplication::setOrganizationName( "avoti" );
    QCoreApplication::setApplicationName( "ketoevent" );

    // set debug level
#ifdef APPLET_DEBUG
    m.setDebugLevel( Common::System );
#else
    m.setDebugLevel( Common::NoDebug );
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
    Gui_Console::init();
#endif

    // init main window
    Gui_Main gui;
    gui.show();

    // initialise application
    if ( m.initialise( qobject_cast<QObject*>( &gui ))) {

        // add teams
        gui.initialise();
    }

    // exec app
    return app.exec();
}
