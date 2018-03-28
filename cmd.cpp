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
#include "cmd.h"
#include "mainwindow.h"
#include <QSqlQuery>
#include <QDebug>

// only available in debugging mode
#ifdef APPLET_DEBUG

/**
 * @brief Cmd::Cmd
 * @param parent
 */
Cmd::Cmd( QObject *parent) : QObject( parent ) {
    auto listCmd = []( const QString &name, const QStringList &args ) { Cmd::instance()->list( name, args ); };

    // add common commands
    this->add( "cmd_list", +listCmd, this->tr( "list all available commands" ));
    this->add( "con_print", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->print( name, args ); }, this->tr( "print text to console" ));
    this->add( "cv_list", +[]( const QString &, const QStringList & ) { Cmd::instance()->listCvars(); }, this->tr( "list all available console variables" ));
    this->add( "cv_set", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->cvarSet( name, args ); }, this->tr( "set console variable value" ));
    this->add( "db_info", +[]( const QString &, const QStringList & ) { Cmd::instance()->dbInfo(); }, this->tr( "display database information" ));
    this->add( "logs_clear", +[]( const QString &, const QStringList & ) { Cmd::instance()->clearLogs(); }, this->tr( "clear logs for current event" ));
    this->add( "combos_clear", +[]( const QString &, const QStringList & ) { Cmd::instance()->clearCombos(); }, this->tr( "clear combos for current event" ));
    this->add( "team_add", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->teamAdd( name, args ); }, this->tr( "add a new team to the current event" ));
    this->add( "team_remove", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->teamRemove( name, args ); }, this->tr( "remove a team" ));
    this->add( "team_logs", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->teamLogs( name, args ); }, this->tr( "print number of logs for a team" ));
    this->add( "shutdown", +[]( const QString &, const QStringList & ) { Main::instance()->shutdown(); }, this->tr( "terminate the applet" ));
    this->add( "sys_stressTest", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->stressTest( name, args ); }, this->tr( "stress test the applet" ));
    this->add( "help", listCmd, this->tr( "same as cmd_list" ));
    this->add( "mem_info", +[]( const QString &, const QStringList & ) {
        qInfo() << QString( "meminfo: %1 allocs, %2 deallocs" )
                       .arg( Main::instance()->alloc )
                       .arg( Main::instance()->dealloc );
    }, this->tr( "print alloc/dealloc count" ));

}

/**
 * @brief Cmd::add
 * @param name
 * @param function
 * @param description
 */
void Cmd::add( const QString &name, function_t function, const QString &description ) {
    // failsafe
    if ( this->contains( name )) {
        qWarning() << this->tr( "command \"%1\" already exists" ).arg( name );
        return;
    }

    // map a new command
    this->functionMap.insert( name, function );
    this->descriptionMap.insert( name, description );
}

/**
 * @brief Cmd::print prints text to console
 * @param args message
 */
void Cmd::print( const QString &name, const QStringList &args ) {
    if ( args.count() < 1 ) {
        qWarning() << this->tr( "usage: %1 [message] - prints text to console" ).arg( name );
        return;
    }

    // just merge everything into a single message
    qInfo() << QString( "%1" ).arg( args.join( " " ));
}

/**
 * @brief Cmd::list lists all available commands
 * @param args filter
 */
void Cmd::list( const QString &, const QStringList &args ) {
    // announce
    if ( !args.isEmpty()) {
        int numFiltered = 0;

        // get total filtered count
        foreach ( const QString &name, this->functionMap.keys()) {
            if ( !args.isEmpty() && !name.startsWith( args.first()))
                continue;
            numFiltered++;
        }

        if ( !numFiltered )
            qWarning() << this->tr( "could not match any available commands" );
        else
            qInfo() << this->tr( "matched %1 of %2 available commands:" ).arg( numFiltered ).arg( this->functionMap.count());
    } else {
        qInfo() <<  this->tr( "%1 available commands:" ).arg( this->functionMap.count());
    }

    foreach ( const QString &name, this->functionMap.keys()) {
        QString description;

        if ( !args.isEmpty() && !name.startsWith( args.first()))
            continue;

        description = this->descriptionMap[name];
        qInfo() << ( !description.isEmpty() ? QString( " %1 - %2" ).arg( name, description ) : QString( " %1" ).arg( name ));
    }
}

/**
 * @brief Cmd::listCvars lists all available console variables
 */
void Cmd::listCvars() {
    if ( !Variable::instance()->list.isEmpty())
        qWarning() << QString( "%1 available console variables:" ).arg( Variable::instance()->list.count());

    foreach ( VariableEntry entry, Variable::instance()->list ) {
        if ( !QString::compare( entry.key(), "system/consoleHistory" ))
            continue;

        if ( QString::compare( entry.defaultValue().toString(), entry.value().toString(), Qt::CaseInsensitive ))
            qInfo() << QString( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( entry.key()).arg( entry.value().toString()).arg( entry.defaultValue().toString());
        else
            qInfo() << QString( "  \"%1\" is \"%2\"" ).arg( entry.key()).arg( entry.value().toString());
    }
}

/**
 * @brief Cmd::cvarSet sets a new value to a console variable
 * @param args key, value
 */
void Cmd::cvarSet( const QString &name, const QStringList &args ) {
    if ( args.count() < 2 ) {
        qWarning() << this->tr( "usage: %1 [key] [value] - set console variable value" ).arg( name );
        return;
    }

    // NOTE: this is case sensitive
    if ( Variable::instance()->contains( args.first())) {
        qInfo() << QString( "no such cvar - \"%1\"" ).arg( args.first());
    } else {
        VariableEntry entry;

        entry = Variable::instance()->list[args.first()];
        qInfo() << QString( "setting \"%1\" to \"%2\"" ).arg( args.at( 1 )).arg( entry.key());
        Variable::instance()->setValue( entry.key(), args.at( 1 ));
    }
}

/**
 * @brief Cmd::dbInfo prints database info to console
 */
void Cmd::dbInfo() {
    int events = 0, teams = 0, tasks = 0, logs = 0;
    QSqlQuery query;

    // print out memory contents
    qInfo() << QString( "MEMORY: events - %1, teams - %2 (%3), tasks - %4 (%5), logs - %6" )
                   .arg( Main::instance()->eventList.count())
                   .arg( Event::active()->teamList.count())
                   .arg( Main::instance()->teamList.count())
                   .arg( Event::active()->taskList.count())
                   .arg( Main::instance()->taskList.count())
                   .arg( Main::instance()->logList.count());

    // print out actual databse contents
    query = QSqlQuery( "select count(*) from events" );
    if ( query.next())
        events = query.value( 0 ).toInt();
    query = QSqlQuery( "select count(*) from teams" );
    if ( query.next())
        teams = query.value( 0 ).toInt();
    query = QSqlQuery( "select count(*) from tasks" );
    if ( query.next())
        tasks = query.value( 0 ).toInt();
    query = QSqlQuery( "select count(*) from logs" );
    if ( query.next())
        logs = query.value( 0 ).toInt();

    qInfo() << QString( "DATABASE: events - %1, teams - %2, tasks - %3, logs - %4" )
                   .arg( events ).arg( teams ).arg( tasks ).arg( logs );
}

/**
 * @brief Cmd::clearLogs clears ALL logs
 */
void Cmd::clearLogs() {
    QSqlQuery query;

    foreach ( Team *team, Event::active()->teamList ) {
        foreach ( Log *log, team->logList ) {
            log->setValue( 0 );
            query.exec( QString( "delete from logs where value=%1" ).arg( log->id()));
        }
    }
}

/**
 * @brief Cmd::clearCombos clears ALL combos
 */
void Cmd::clearCombos() {
    foreach ( Team *team, Event::active()->teamList ) {
        foreach ( Log *log, team->logList )
            log->setComboId( -1 );
    }
}

/**
 * @brief Cmd::teamAdd adds a new team to current event
 * @param args name, members
 */
void Cmd::teamAdd( const QString &name, const QStringList &args ) {
    if ( args.count() < 2 ) {
        qWarning() << this->tr( "usage: %1 [name] [members] - add a new team to the current event" ).arg( name );
        return;
    }
    Team::add( args.at( 0 ), args.at( 1 ).toInt(), Event::active()->startTime(), Variable::instance()->string( "reviewerName" ), false );
}

/**
 * @brief Cmd::teamRemove removes a team from current event
 * @param args name
 */
void Cmd::teamRemove( const QString &name, const QStringList &args ) {
    if ( args.count() < 1 ) {
        qInfo() << this->tr( "usage: %1 [name] - remove team" ).arg( name );
        return;
    }

    if ( Event::active()->teamList.indexOf( Team::forName( args.at( 0 ))) != -1 )
        Team::remove( args.at( 0 ));
}

/**
 * @brief Cmd::teamLogs removes a team from current event
 * @param args name
 */
void Cmd::teamLogs( const QString &name, const QStringList &args ) {
    if ( args.count() < 1 ) {
        qWarning() << this->tr( "usage: %1 [name] - number of logs for a team" ).arg( name );
        return;
    }

    Team *team = Team::forName( args.at( 0 ));
    int count = 0;
    if ( Event::active()->teamList.indexOf( team ) != -1 ) {
        foreach ( Log *log, Main::instance()->logList ) {
            if ( log->teamId() == team->id())
                count++;
        }
        qInfo() << this->tr( "  %1 logs" ).arg( count );
    } else
        qInfo() << this->tr( "  team  \"%1\" not found" ).arg( args.at( 0 ));
}

/**
 * @brief Cmd::stressTest log imitation for debugging purposes
 * @param args stress test options (see source code)
 */
void Cmd::stressTest( const QString &name, const QStringList &args ) {
    MainWindow *mainWindow;

    enum ComboCount {
        C0 = 0,
        C2,
        C23,
        C234
    };

    if ( args.count() < 1 ) {
        qWarning() << this->tr( "usage: %1 [numTeams] - stress test the applet" ).arg( name );
        return;
    }

    mainWindow = qobject_cast<MainWindow *>( Main::instance()->parent());
    if ( !QString::compare( args.first(), "clear" )) {
        mainWindow->stressTest( -1 );
        return;
    }

    if ( !QString::compare( args.first(), "custom" )) {
        mainWindow->stressTest( -2 );
        return;
    }

    if ( !QString::compare( args.first(), "combos" )) {
        foreach ( Team *team, Event::active()->teamList ) {
            int numLogs = 0;

            foreach ( Log *log, team->logList ) {
                if ( log->value() == 0 )
                    continue;

                // do something
                numLogs++;
            }

            ComboCount comboCount = C0;

            // don't like hard coding, but for stress test this should be ok
            if ( numLogs >= 3 && numLogs < 8 )
                comboCount = C2;
            else if ( numLogs >= 8 && numLogs < 11 )
                comboCount = C23;
            else if ( numLogs >= 11 )
                comboCount = C234;

            QList <Log*> logList;

            foreach ( Log *log, team->logList ) {
                if ( log->value() == 0 )
                    continue;

                if ( comboCount == C0 )
                    break;

                // first fill the largest pool
                logList << log;

                if ( comboCount == C234 && logList.count() == 4 )
                    comboCount = C23;
                else if ( comboCount == C23 && logList.count() == 3 )
                    comboCount = C2;
                else if ( comboCount == C2 && logList.count() == 2 )
                    comboCount = C0;
                else
                    continue;

                int freeHandle = Combo::getFreeHandle();
                foreach ( Log *log, logList )
                    log->setComboId( freeHandle );

                logList.clear();
            }
        }
        return;
    }

    if ( mainWindow != nullptr )
        mainWindow->stressTest( args.first().toInt());
}

/**
 * @brief Cmd::executeTokenized subdivides command string into args and executes it
 * @param command command name
 * @param args argument string list
 * @return success
 */
bool Cmd::executeTokenized( const QString &name, const QStringList &args ) {
    if ( this->contains( name )) {
        this->functionMap[name]( name, args );
        return true;
    }

    // find the cvar
    if ( Variable::instance()->contains( name )) {
        VariableEntry entry;

        entry = Variable::instance()->list[name];
        if ( args.count() >= 1 ) {
            QStringList cvCmd;
            cvCmd.append( name );
            cvCmd << args;
            this->cvarSet( name, cvCmd );
        } else
            qInfo() << QString( "%1" ).arg( entry.value().toString());

        return true;
    }

    // report unknown command
    qWarning() << this->tr( "unknown command \"%1\", prehaps try \"cmd_list\"" ).arg( name );
    return false;
}

/**
 * @brief Cmd::tokenize tokenizes command string
 * @param string command string
 * @param command command name
 * @param arguments arguments
 * @return
 */
bool Cmd::tokenize( const QString &string, QString &command, QStringList &args ) {
    int pos = 0, len;
    QString capture;
    QRegExp rx;

    // make sure input is blank
    command.clear();
    args.clear();

    // set capture pattern
    rx.setPattern( "((?:[^\\s\"]+)|(?:\"(?:\\\\\"|[^\"])*\"))" );

    // tokenize the string
    while (( pos = rx.indexIn( string, pos )) != -1 ) {
        capture = rx.cap( 1 );
        len = rx.matchedLength();

        // the first one should be the command
        if ( command.isEmpty()) {
            command = capture;
            pos += len;
            continue;
        }

        // then follow the arguments
        // make sure we remove extra quotes
        if ( capture.startsWith( "\"" ) || capture.endsWith( "\"" )) {
            capture.remove( 0, 1 );
            capture.remove( capture.length()-1, 1 );
        }
        args.append( capture );
        pos += len;
    }

    if ( command.isEmpty())
        return false;

    return true;
}

/**
 * @brief Cmd::execute executes command
 * @param buffer command string from console
 * @return success
 */
bool Cmd::execute( const QString &buffer ) {
    int counter = 0;
    QString command;
    QStringList arguments, separated;

    // separate multiline commands first
    separated = buffer.split( QRegExp( ";|\\n" ));

    // parse separated command strings
    foreach ( QString string, separated ) {
        // tokenize & execute command
        if ( this->tokenize( string, command, arguments ))
            counter += this->executeTokenized( command, arguments );
    }

    if ( counter )
        return true;

    return false;
}

#endif
