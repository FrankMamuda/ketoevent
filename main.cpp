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
#include <QDir>
#include <QSqlError>
#include <QMessageBox>
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
- add compatibility layer for the 2013 event (or just stats)
- use homedir for mac (if applicable)
- combos for stressTest
- gui list of tasks with zero logs
- richText styling? html?
- unify task and team dialogs (create superclass - less dup code)
*/

/**
 * @brief Main::initialise
 * @param parent
 */
bool Main::initialise( QObject *parent ) {
    // announce
    m.print( StrMsg + this->tr( "initialising system\n" ), Main::System );

    // set parent
    this->setParent( parent );

    // init counters
    this->changesCounter = 0;
    this->setInitialised( false );

    // initialise settings
    this->settings = new QSettings( "avoti", "ketoevent" );
    this->settings->setDefaultFormat( QSettings::NativeFormat );

    // initialise this first
    this->addCvar( new ConsoleVariable( "databasePath", this->settings, this->path ));

    // make default path
    this->makePath( this->cvar( "databasePath" )->string());
    this->cvar( "databasePath" )->setValue( this->path );

    // init cvars
    this->addCvar( new ConsoleVariable( "backup/perform", this->settings, true ));
    this->addCvar( new ConsoleVariable( "backup/changes", this->settings, 25 ));
    this->addCvar( new ConsoleVariable( "misc/sortTasks", this->settings, true ));
    this->addCvar( new ConsoleVariable( "misc/hilightLogged", this->settings, false ));
    this->addCvar( new ConsoleVariable( "currentEvent", this->settings, -1 ));
    this->addCvar( new ConsoleVariable( "reviewerName", this->settings, "" ));
    this->addCvar( new ConsoleVariable( "system/consoleHistory", this->settings, "" ));
    this->addCvar( new ConsoleVariable( "rankings/current", this->settings, true ));

    // add an empty car
    this->defaultCvar = new ConsoleVariable( "default", this->settings, false );
    this->defaultSvar = new SettingsVariable( "default", SettingsVariable::NoType, SettingsVariable::NoClass );

    // load database entries
    if ( !this->loadDatabase())
        return false;

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
    this->addSvar( "misc/hilightLogged", SettingsVariable::CheckBox, SettingsVariable::ConsoleVar );
    this->addSvar( "name", SettingsVariable::LineEdit, SettingsVariable::EventVar );
    this->addSvar( "databasePath", SettingsVariable::LineEdit, SettingsVariable::ConsoleVar );
    this->addSvar( "reviewerName", SettingsVariable::LineEdit, SettingsVariable::ConsoleVar );
    this->addSvar( "rankings/current", SettingsVariable::Action, SettingsVariable::ConsoleVar );

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
 * @brief Main::print
 * @param msg
 * @param debug
 */
void Main::print( const QString &msg, DebugLevel debug ) {
    QString out = msg;

    if ( msg.endsWith( "\n" ))
        out = msg.left( msg.length()-1 );

#ifdef APPLET_DEBUG
    // EVERYTHING is printed to the console
    if ( this->console != NULL )
        this->console->print( out );

    // subsystem messages may be skipped
    if ( debug == Main::NoDebug )
        return;
    else {
        if ( !this->debugLevel().testFlag( debug ))
            return;
    }

    // output to QDebug
    qDebug() << out;
#else
    Q_UNUSED( msg );
#endif
}

/**
 * @brief Main::error
 * @param type
 * @param func
 * @param msg
 */
void Main::error( ErrorTypes type, const QString &func, const QString &msg ) {
    Gui_Main *guiPtr;
    guiPtr = qobject_cast<Gui_Main*>( this->parent());
    QString dialogMsg;

    // capitalize for message boxes
    dialogMsg = msg;
    dialogMsg.replace( 0, 1, dialogMsg.at( 0 ).toUpper());

    if ( type == FatalError ) {
        this->print( this->tr( "FATAL ERROR: %1" ).arg( func + msg ), System );

        if ( guiPtr != NULL ) {
            guiPtr->lock();
            QMessageBox msgBox;
            msgBox.setWindowTitle( this->tr( "Fatal error" ));
            msgBox.setText( dialogMsg + "\n" + this->tr( "Do you want to reset the database (requires restart)?" ));
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msgBox.setIcon( QMessageBox::Critical );
            int state = msgBox.exec();

            // check options
            switch ( state ) {
            case QMessageBox::Yes:
                m.unloadDatabase();
                QFile::rename( m.cvar( "databasePath" )->string(), QString( "%1_badDB_%2.db" ).arg( m.cvar( "databasePath" )->string().remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
                guiPtr->close();
                break;

            case QMessageBox::No:
            default:
                ;
            }
        } else {
            exit( 0 );
        }
    } else {
        if ( guiPtr != NULL )
            QMessageBox::warning( guiPtr, "Error", dialogMsg, QMessageBox::Close );

        this->print( this->tr( "ERROR: %1" ).arg( func + msg ), System );
    }
}

/**
 * @brief Main::shutdown perform garbage collection
 * @param ignoreDatabase
 */
void Main::shutdown( bool ignoreDatabase ) {    
    // announce
    m.print( StrMsg + this->tr( "performing shutdown\n" ), Main::System );

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
    #ifdef APPLET_DEBUG
        this->console->close();
    #endif

        // close database
        if ( !ignoreDatabase )
            this->unloadDatabase();

        // reset initialisation state
        this->setInitialised( false );

        // close applet
        QApplication::quit();
    } else {
        m.unloadDatabase();
        exit( 0 );
    }
}

/**
 * @brief Main::writeBackup
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
            this->error( StrFatalError, this->tr( "could not create backup path\n" ));
            return;
        }
    }
    QFile::copy( this->path, QString( "%1%2_%3.db" ).arg( backupPath ).arg( QFileInfo( this->path ).fileName().remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
}

/**
 * @brief Main::update
 */
void Main::update() {
    this->changesCounter++;

    if ( this->changesCounter == this->cvar( "backup/changes" )->value().toInt() && this->cvar( "backup/perform" )->value().toBool()) {
        this->writeBackup();
        this->changesCounter = 0;
    }
}

#ifdef APPLET_DEBUG
/**
 * @brief Main::initConsole
 */
void Main::initConsole() {
    // announce
    m.print( StrMsg + this->tr( "initilising console\n" ), Main::System );

    if ( this->console != NULL )
        return;

    this->console = new Gui_Console();
    this->console->hide();
}
#endif

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
    foreach ( Team *teamPtr, this->base.teamList ) {
        this->base.teamList.removeOne( teamPtr );
        delete teamPtr;
    }
    this->base.teamList.clear();

    // clear events
    foreach ( Event *eventPtr, this->base.eventList ) {
        this->base.eventList.removeOne( eventPtr );
        delete eventPtr;
    }
    this->base.eventList.clear();

    // clear tasks
    foreach ( Task *taskPtr, this->base.taskList ) {
        this->base.taskList.removeOne( taskPtr );
        delete taskPtr;
    }
    this->base.taskList.clear();
    //this->taskListSorted().clear();
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
    m.setDebugLevel( Main::Debug );
    //m.setDebugLevel( Main::DebugLevel( 0x00FF ));
#else
    m.setDebugLevel( Main::NoDebug );
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
    m.initConsole();
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
