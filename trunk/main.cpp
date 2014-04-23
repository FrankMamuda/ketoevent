/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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
// preload events
// set name in titlebar
// allow locking whole events
// db paths in settings
// statistics - import void teams from csv (just to see the results)
// check private/public funcs
// check includes
// add more props
// verbocity for console Regular Verbose PrintEverything
// advanced combos
// replace event with bad API
// allow debugging of components:
//       Tasks, Teams, Gui as flags (prints out if enabled)
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

    // init cvars
    this->addCvar( new ConsoleVariable( "backup/perform", this->settings, true ));
    this->addCvar( new ConsoleVariable( "backup/changes", this->settings, 25 ));
    this->addCvar( new ConsoleVariable( "misc/sortTasks", this->settings, false ));
    this->addCvar( new ConsoleVariable( "lastEventId", this->settings, -1 ));

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
    this->addSvar( "singleCombo", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "doubleCombo", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "tripleCombo", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "minMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "maxMembers", SettingsVariable::SpinBox, SettingsVariable::EventVar );
    this->addSvar( "backup/changes", SettingsVariable::SpinBox, SettingsVariable::ConsoleVar );
    this->addSvar( "backup/perform", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    this->addSvar( "misc/sortTasks", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    this->addSvar( "name", SettingsVariable::LineEdit, SettingsVariable::EventVar );

    // set parent
    this->parent = parent;

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
================
*/
void Main::shutdown( bool ignoreDatabase ) {
    // save settings
    if ( this->settings != NULL ) {
        this->settings->sync();
        delete this->settings;
    }

    // delete orphaned logs on shutdown
    if ( !ignoreDatabase ) {
        // close database if open
        QSqlDatabase db = QSqlDatabase::database();
        if ( db.open()) {
            this->removeOrphanedLogs();
            //this->removeOrphanedCombos();
            db.close();
        }
    }

    // clear console vars
    foreach ( ConsoleVariable *varPtr, this->cvarList )
        delete varPtr;
    this->cvarList.clear();

    if ( this->defaultCvar != NULL )
        delete this->defaultCvar;

    // garbage collection
    foreach ( SettingsVariable *varPtr, this->svarList )
        delete varPtr;
    this->svarList.clear();

    if ( this->defaultSvar != NULL )
        delete this->defaultSvar;

    if ( this->console != NULL )
        delete this->console;

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

    // make path id nonexistant
    backupPath = this->path + "backups/";
    QDir dir( backupPath );
    if ( !dir.exists()) {
        dir.mkpath( backupPath );
        if ( !dir.exists()) {
            m.error( StrFatalError + this->tr( "could not create backup path\n" ));
            return;
        }
    }
    QFile::copy( this->databasePath, QString( "%1%2.db" ).arg( backupPath ).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
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
//#ifndef APPLET_DEBUG
    m.initialize( qobject_cast<QObject*>( &gui ));
//#endif

    // add teams
    gui.initialize();

    // exec app
    return app.exec();
}
