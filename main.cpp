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
#include <QMessageBox>
#include <QTranslator>
#include "console.h"
#include "cmd.h"
#include "xmltools.h"

/*
TODO:
- models for all comboBoxes
    where database changes (add/edit/load/remove)
    triggers update (model reset)
- remove orphaned logs on recalculation
  also check for duplicates in team->logList
- remove Dialog class
- lambdas, overrides, singletons
- threading (database, rankings)
- qDebug() context
- database code rewrite (singleton)
- change icons
- store last teamId

FUTURE:
- use listView instead of listWidget to reduce memory footprint
- use homedir for mac (if applicable)
- richText styling? html?
- unify task and team dialogs (create superclass - less dup code)
- completers everywhere (find, etc.)

NOTES:
- currently, there is no need for an API change
- sort teams alphabetically?
*/

/**
 * @brief Main::initialise
 * @param parent
 */
bool Main::initialise( QObject *parent ) {
    // announce
    qInfo() << this->tr( "initialising system" );

    // init cvars
    Variable::instance()->add( "databasePath", this->path, Var::ReadOnly );
    Variable::instance()->add( "backup/perform", true );
    Variable::instance()->add( "backup/changes", 25 );
    Variable::instance()->add( "misc/sortTasks", true );
    Variable::instance()->add( "misc/hilightLogged", false );
    Variable::instance()->add( "currentEvent", -1, ( Var::Hidden | Var::ReadOnly ));
    Variable::instance()->add( "reviewerName", "" );
    Variable::instance()->add( "system/consoleHistory", "", Var::Hidden );
    Variable::instance()->add( "rankings/current", true, Var::Hidden );

    // add custom vars
    Variable::instance()->add<EventVariable>( "name", this->tr( "Custom event" ), Var::NoSave );
    Variable::instance()->add<EventVariable>( "startTime", KetoEvent::defaultStartTime, Var::NoSave );
    Variable::instance()->add<EventVariable>( "finishTime", KetoEvent::defaultFinishTime, Var::NoSave );
    Variable::instance()->add<EventVariable>( "finalTime", KetoEvent::defaultFinalTime, Var::NoSave );
    Variable::instance()->add<EventVariable>( "penalty", KetoEvent::defaultPenaltyPoints, Var::NoSave );
    Variable::instance()->add<EventVariable>( "comboOfTwo", KetoEvent::defaultComboOfTwo, Var::NoSave );
    Variable::instance()->add<EventVariable>( "comboOfThree", KetoEvent::defaultComboOfThree, Var::NoSave );
    Variable::instance()->add<EventVariable>( "comboOfFourAndMore", KetoEvent::defaultComboOfFourAndMore, Var::NoSave );
    Variable::instance()->add<EventVariable>( "minMembers", KetoEvent::defaultMinMembers, Var::NoSave );
    Variable::instance()->add<EventVariable>( "maxMembers", KetoEvent::defaultMaxMembers, Var::NoSave );

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
    qInfo() << this->tr( "performing shutdown" );

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
    MainWindow *mainWindow;

    // get main screen
    mainWindow = qobject_cast<MainWindow*>( this->parent());
    if ( mainWindow == nullptr )
        return;

    // clear task widgets on main screen
    mainWindow->clearTasks();

    // clear teams (logs should be cleaned automatically on destruct)
    foreach ( Team *team, Main::instance()->teamList ) {
        Main::instance()->teamList.removeOne( team );
        delete team;
    }
    Main::instance()->teamList.clear();

    // clear events
    foreach ( Event *event, Main::instance()->eventList ) {
        Main::instance()->eventList.removeOne( event );
        delete event;
    }
    Main::instance()->eventList.clear();

    // clear tasks
    foreach ( Task *task, Main::instance()->taskList ) {
        Main::instance()->taskList.removeOne( task );
        delete task;
    }
    Main::instance()->taskList.clear();
}

/**
 * @brief Main::Main
 * @param parent
 */
Main::Main( QObject *parent) : QObject( parent ),
    alloc( 0 ), dealloc( 0 ),
    console( nullptr ), teamModel( new TeamListModel()),
    m_init( false ) {}

/**
 * @brief Main::~Main
 */
Main::~Main() { delete this->teamModel; }

// default message handler
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler( 0 );

/**
 * @brief messageFilter
 * @param type
 * @param context
 * @param msg
 */
void messageFilter( QtMsgType type, const QMessageLogContext &context, const QString &msg ) {
    (*QT_DEFAULT_MESSAGE_HANDLER)( type, context, msg );

#ifdef APPLET_DEBUG
    // EVERYTHING is printed to the console
    if ( Main::instance()->console != nullptr ) {
        QString out( msg );

        out = out.remove( "\\" );

        if ( out.startsWith( "\"" ) && out.endsWith( "\"" ))
            out = out.remove( 0, 1 ).remove( out.length() - 2, 1 );;

        Main::instance()->console->print( /*context.function +*/ out );

    }
#endif

    if ( type == QtFatalMsg ) {
        MainWindow *mainWindow;

        mainWindow = qobject_cast<MainWindow*>( Main::instance()->parent());
        if ( mainWindow != nullptr ) {
            mainWindow->lock();
            QMessageBox msgBox;
            msgBox.setWindowTitle( QObject::tr( "Fatal error" ));
            msgBox.setText( msg + "\n" + QObject::tr( "Do you want to reset the database (requires restart)?" ));
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msgBox.setIcon( QMessageBox::Critical );
            int state = msgBox.exec();

            // check options
            switch ( state ) {
            case QMessageBox::Yes:
                Database::unload();
                QFile::rename( Variable::instance()->string( "databasePath" ), QString( "%1_badDB_%2.db" ).arg( Variable::instance()->string( "databasePath" ).remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
                mainWindow->close();
                break;

            case QMessageBox::No:
            default:
                ;
            }
        }

        QApplication::quit();
        exit( 0 );
    }
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    // set console output pattern
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );

    // log to file in non-qtcreator environment
    qInstallMessageHandler( messageFilter );

    // init app
    QApplication app( argc, argv );
    QCoreApplication::setOrganizationName( "factory12" );
    QCoreApplication::setApplicationName( "ketoevent" );

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
