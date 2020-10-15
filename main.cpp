/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QDesktopWidget>
#include <QTranslator>
#include <QSettings>
#include "theme.h"
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
// GENERAL TODO/FIXME LIST
//
//   - scripted states (in a separate branch)
//   - upon fresh start, tasks are unavailable unless a team is added
//   - more self tests
//   - ideally info button would align itself to the left
//   - new combo should not start with 1
//   - task export
//

// default message handler
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler( nullptr );

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

    // simple single instance implementation
    // NOTE: this however will fail if app crashes during startup
#ifndef QT_DEBUG
    class SharedMemory : public QSharedMemory {
    public:
        SharedMemory( const QString &key, QObject *parent = nullptr ) : QSharedMemory( key, parent ) {}
        ~SharedMemory() { if ( this->isAttached()) { this->detach(); } }

        bool lock() {
            if ( this->isAttached()) return false;
            if ( this->attach( QSharedMemory::ReadOnly )) { this->detach(); return false; }
            return this->create( sizeof( quint64 ));
        }
    };
    QSharedPointer<SharedMemory> sharedMemory( new SharedMemory( "ketoevent_singleInstance", &a ));
    if ( !sharedMemory->lock())
        return 0;
#endif

    // register metatypes
    qRegisterMetaType<Item::Actions>();
    qRegisterMetaType<Item::Types>();
    qRegisterMetaType<Event::Fields>();
    qRegisterMetaType<Log::Fields>();
    qRegisterMetaType<Id>();
    qRegisterMetaType<Row>();
    qRegisterMetaType<RankingsModel::Columns>();
    qRegisterMetaType<Table::Roles>();
    qRegisterMetaType<Task::Fields>();
    qRegisterMetaType<Task::Types>();
    qRegisterMetaType<Task::Styles>();
    qRegisterMetaType<Team::Fields>();
    //qRegisterMetaType<Var::Flags>();
    qRegisterMetaType<Widget::Types>();

    // set console output pattern
#ifdef QT_DEBUG
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );
#else
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{message}" );
#endif

    // log to file in non-qtcreator environment
    qInstallMessageHandler( messageFilter );

    // i18n
    QTranslator translator;
#ifndef FORCE_LV_LOCALE
    const QString locale( QLocale::system().name());
#else
    const QString locale( "lv_LV" );
#endif
    QLocale::setDefault( locale );
    translator.load( ":/i18n/ketoevent_" + locale );
    QApplication::installTranslator( &translator );

    // set variable defaults
    Variable::add( "reviewerName", "" );
    Variable::add( "eventId", -1, Var::Flag::Hidden );
    Variable::add( "teamId", -1, Var::Flag::Hidden );
    Variable::add( "rankingsCurrent", true );
    Variable::add( "sortByType", true );
    Variable::add( "system/consoleHistory", "", Var::Flag::Hidden );
    Variable::add( "databasePath", "", Var::Flag::Hidden );
    Variable::add( "backup/enabled", false );
    Variable::add( "backup/changes", 25 );
    Variable::add( "darkMode", false, Var::Flag::ReadOnly | Var::Flag::Hidden | Var::Flag::NoSave );
    Variable::add( "overrideTheme", false, Var::Flag::ReadOnly | Var::Flag::Hidden );
    Variable::add( "theme", "light", Var::Flag::ReadOnly | Var::Flag::Hidden );

    // read configuration
    XMLTools::read();

    // check for previous crashes
    const QString apiFileName( QDir::currentPath() + "/badapi" );
    if ( QFileInfo::exists( apiFileName )) {
        const QFileInfo info( Variable::string( "databasePath" ));

        // just change path
        Variable::setString( "databasePath", info.absolutePath() + "/database_"
                                                         + QDateTime::currentDateTime()
                                                                 .toString( "yyyyMMdd_hhmmss" ) +
                                                         ".db" );
        // reset vars
        Variable::reset( "eventId" );
        Variable::reset( "teamId" );

        // copy built-in demo version
        //QFile::copy( ":/initial/database.db", Variable::string( "databasePath" ));
        //QFile( Variable::string( "databasePath" )).setPermissions(
        //        QFileDevice::ReadOwner | QFileDevice::WriteOwner );

        QFile::remove( apiFileName );
    }


    // initialize database and its tables
    Database::instance();
    auto loadTables = []() {
        bool success = true;

        // initialize database and its tables
        success &= Database::instance()->add( Event::instance());
        success &= Database::instance()->add( Task::instance());
        success &= Database::instance()->add( Team::instance());
        success &= Database::instance()->add( Log::instance());

        return success;
    };

    if ( !loadTables()) {
        QMessageBox::critical( QApplication::desktop(),
                               QObject::tr( "Internal error" ),
                               QObject::tr( "Could not load database\n"
                                            "New database will be created\n"
                                            "Please restart the application" ),
                               QMessageBox::Ok );

        QFile badAPIFile( apiFileName );
        if ( badAPIFile.open( QIODevice::WriteOnly ))
            badAPIFile.close();

        QApplication::quit();
        return 0;
    }
    // detect dark mode
    bool darkMode = false;
    bool darkModeWin10 = false;
#ifdef Q_OS_WIN
    const QVariant key( QSettings( R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
                                   QSettings::NativeFormat ).value( "AppsUseLightTheme" ));
    if ( key.isValid() && !key.toBool()) {
        darkMode = true;
        darkModeWin10 = true;

        if ( !Variable::isEnabled( "overrideTheme" ))
            Variable::setString( "theme", "dark" );
    }
#else
    if ( qGray( QApplication::palette().color( QPalette::Base ).rgb()) < 128 )
        darkMode = true;
#endif

    // set default icon theme
    QIcon::setThemeName( darkMode ? "dark" : "light" );

    if ( Variable::isEnabled( "overrideTheme" ) || darkModeWin10 ) {
        // load theme from file
        auto *theme( new Theme( Variable::string( "theme" )));

        // override the variable
        Variable::setEnabled( "darkMode", theme->isDark());

        // override style and palette
        QApplication::setStyle( theme->style());
        QApplication::setPalette( theme->palette());

        // override icon theme and syntax highlighter theme
        QIcon::setThemeName( theme->isDark() ? "dark" : "light" );
        //MainWindow::instance()->setTheme( theme );
    }

    // show main window
    MainWindow::instance()->show();
    Task::instance()->setInitialised();

    // reset tasks after task table initialization
    MainWindow::instance()->setTaskFilter();

    // initialize console
    Main::Console = Console::instance();

    // clean up on exit
    qApp->connect( qApp, &QApplication::aboutToQuit, []() {
        Task::instance()->setInitialised( false );

        delete Console::instance();
        Main::Console = nullptr;

        XMLTools::write();
        GarbageMan::instance()->clear();

        delete GarbageMan::instance();

        if ( Database::instance() != nullptr )
            delete Database::instance();

        delete Variable::instance();
        // FIXME: delete MainWindow::instance();
    } );

    return a.exec();
}
