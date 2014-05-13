/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// defines
//
//#define FORCE_LATVIAN
#define APPLET_DEBUG

//
// includes
//
#include "main.h"
#include "gui_main.h"
#include <QApplication>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTranslator>
#include "gui_console.h"
#ifdef APPLET_DEBUG
#include <QDebug>
#endif

//
// classes
//
class Main m;

//
// TODO:
//
// deprecate db importing, allow only import of statistics
//   (or at least calculate stats on the fly (no real allocs))
// statistics - import void teams from csv (just to see the results)
// check private/public funcs
// check includes
// const and static funcs
// verbocity for console Regular Verbose PrintEverything
// replace event with bad API
// allow debugging of components:
//       Tasks, Teams, Gui as flags (prints out if enabled)
// TODO: store last EVENT!!! in settings
//

/*
================
initialize
================
*/
void Main::initialize( QObject *parent ) {
    // init counters
    this->changesCounter = 0;
    this->setInitialized( false );

    // initialize settings
    this->settings = new QSettings( "avoti", "ketoevent3" );
    this->settings->setDefaultFormat( QSettings::NativeFormat );

    // initialize this first
    this->addCvar( new ConsoleVariable( "databasePath", this->settings, this->path ));

    // make default path
    this->makePath( this->cvar( "databasePath" )->string());

    // init cvars
    this->addCvar( new ConsoleVariable( "backup/perform", this->settings, true ));
    this->addCvar( new ConsoleVariable( "backup/changes", this->settings, 25 ));
    this->addCvar( new ConsoleVariable( "misc/sortTasks", this->settings, false ));
    this->addCvar( new ConsoleVariable( "currentEvent", this->settings, -1 ));

    // add an empty car
    this->defaultCvar = new ConsoleVariable( "default", this->settings, false );
    this->defaultSvar = new SettingsVariable( "default", SettingsVariable::NoType, SettingsVariable::NoClass );

    // load database entries
    // skip this for now
    this->loadDatabase();

    // create settings variables
    this->addSvar( "startTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    this->addSvar( "finishTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    this->addSvar( "finalTime", SettingsVariable::TimeEdit, SettingsVariable::EventVar );
    this->addSvar( "penalty", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "comboOfTwo", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "comboOfThree", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "comboOfFourAndMore", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "minMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "maxMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "backup/changes", SettingsVariable::SpinBox, SettingsVariable::ConsoleVar );
    this->addSvar( "backup/perform", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    this->addSvar( "misc/sortTasks", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    this->addSvar( "name", SettingsVariable::LineEdit, SettingsVariable::EventVar );
    this->addSvar( "databasePath", SettingsVariable::LineEdit, SettingsVariable::ConsoleVar );

    // set parent
    this->setParent( parent );

    // we're up
    this->setInitialized();
}

/*
============
print
============
*/
void Main::print( const QString &msg ) {
    // print to console
    if ( this->console != NULL ) {
        if ( msg.endsWith( "\n" ))
            this->console->print( msg.left( msg.length()-1 ));
        else
            this->console->print( msg );
    }

#ifdef APPLET_DEBUG
    qDebug() << msg;
#endif
}

/*
============
error
============
*/
void Main::error( ErrorTypes type, const QString &msg ) {
    if ( type == FatalError ) {
        this->print( this->tr( "FATAL ERROR: %1" ).arg( msg ));
        this->shutdown( true );
    } else
        this->print( this->tr( "ERROR: %1" ).arg( msg ));
}

/*
================
shutdown

  garbage collection
================
*/
void Main::shutdown( bool ignoreDatabase ) {
    // clear parent
    this->setParent( NULL );

    // save settings
    if ( this->settings != NULL ) {
        this->settings->sync();
        delete this->settings;
    }

    // clear entries
    this->clearEvent();

    // clear console vars
    foreach ( ConsoleVariable *varPtr, this->cvarList )
        delete varPtr;
    this->cvarList.clear();
    if ( this->defaultCvar != NULL )
        delete this->defaultCvar;

    // ckear settings vars
    foreach ( SettingsVariable *varPtr, this->svarList )
        delete varPtr;
    this->svarList.clear();
    if ( this->defaultSvar != NULL )
        delete this->defaultSvar;

    // close console
    this->console->close();

    // close database
    if ( !ignoreDatabase )
        this->unloadDatabase();

    // reset initialization state
    this->setInitialized( false );

    // close applet
    QApplication::quit();
}

/*
================
writeBackup
================
*/
void Main::writeBackup() {
    QString backupPath;

    // make path if nonexistant
    QFileInfo db( this->path );
    backupPath = db.absolutePath() + "/" + "backups/";
    QDir dir( backupPath );
    if ( !dir.exists()) {
        dir.mkpath( backupPath );
        if ( !dir.exists()) {
            this->error( StrFatalError + this->tr( "could not create backup path\n" ));
            return;
        }
    }
    QFile::copy( this->path, QString( "%1%2_%3.db" ).arg( backupPath ).arg( QFileInfo( this->path ).fileName().remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
}

/*
================
update
================
*/
void Main::update() {
    this->changesCounter++;

    if ( this->changesCounter == this->cvar( "backup/changes" )->value().toInt() && this->cvar( "backup/perform" )->value().toBool()) {
        this->writeBackup();
        this->changesCounter = 0;
    }
}

/*
================
initConsole
================
*/
void Main::initConsole() {
    if ( this->console != NULL )
        return;

    this->console = new Gui_Console();
    this->console->show();
}

/*
================
clearEvent

  garbage collection
================
*/
void Main::clearEvent() {
    // get main screen
    Gui_Main *gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui == NULL )
        return;

    // clear task widgets on main screen
    gui->clearTasks();

    // clear teams (logs should be cleaned automatically on destruct)
    foreach ( TeamEntry *teamPtr, this->teamList ) {
        this->teamList.removeOne( teamPtr );
        delete teamPtr;
    }
    this->teamList.clear();

    // clear events
    foreach ( EventEntry *eventPtr, this->eventList ) {
        this->eventList.removeOne( eventPtr );
        delete eventPtr;
    }
    this->eventList.clear();

    // clear tasks
    foreach ( TaskEntry *taskPtr, this->taskList ) {
        this->taskList.removeOne( taskPtr );
        delete taskPtr;
    }
    this->taskList.clear();
    this->taskListSorted().clear();
}

/*
================
entry point
================
*/
int main( int argc, char *argv[] ) {
    QApplication app( argc, argv );

    // init app
    QCoreApplication::setOrganizationName( "avoti" );
    QCoreApplication::setApplicationName( "ketoevent3" );

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
    m.initConsole();
#endif

    // init main window
    Gui_Main gui;
#ifdef Q_OS_ANDROID
    app.setStyleSheet( "QTimeEdit { width: 128px; } QTimeEdit::up-button { subcontrol-position: right; width: 32px; height: 32px; } QTimeEdit::down-button { subcontrol-position: left; width: 32px; height: 32px; } QSpinBox { width: 96px; } QSpinBox::up-button { subcontrol-position: right; width: 32px; height: 32px; } QSpinBox::down-button { subcontrol-position: left; width: 32px; height: 32px; } QCheckBox::indicator { width: 32px; height: 32px; }" );
    gui.showMaximized();
#else
    gui.show();
#endif

    // initialize application
    m.initialize( qobject_cast<QObject*>( &gui ));

    // add teams
    gui.initialize();

    // exec app
    return app.exec();
}
