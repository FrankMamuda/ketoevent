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

// only available in debugging mode
#ifdef APPLET_DEBUG

//
// commands
//
createCommandPtr( Cmd::instance(), print )
createCommandPtr( Cmd::instance(), list )
createCommandPtr( Cmd::instance(), cvarSet )
createCommandPtr( Cmd::instance(), teamAdd )
createCommandPtr( Cmd::instance(), teamRemove )
createCommandPtr( Cmd::instance(), teamLogs )
createCommandPtr( Cmd::instance(), stressTest )
createSimpleCommandPtr( Cmd::instance(), dbInfo )
createSimpleCommandPtr( Cmd::instance(), clearLogs )
createSimpleCommandPtr( Cmd::instance(), clearCombos )
createSimpleCommandPtr( Cmd::instance(), listCvars )
createSimpleCommandPtr( Main::instance(), shutdown )
#ifdef APPLET_DEBUG
createSimpleCommandPtr( Cmd::instance(), memInfo )
#endif

/**
 * @brief Cmd::init initialises command subsystem
 */
void Cmd::init() {
    // add common commands
    this->add( "cmd_list", listCmd, this->tr( "list all available commands" ));
    this->add( "con_print", printCmd, this->tr( "print text to console" ));
    this->add( "cv_list", listCvarsCmd, this->tr( "list all available console variables" ));
    this->add( "cv_set", cvarSetCmd, this->tr( "set console variable value" ));
    this->add( "db_info", dbInfoCmd, this->tr( "display database information" ));
    this->add( "logs_clear", clearLogsCmd, this->tr( "clear logs for current event" ));
    this->add( "combos_clear", clearCombosCmd, this->tr( "clear combos for current event" ));
    this->add( "team_add", teamAddCmd, this->tr( "add a new team to the current event" ));
    this->add( "team_remove", teamRemoveCmd, this->tr( "remove a team" ));
    this->add( "team_logs", teamLogsCmd, this->tr( "print number of logs for a team" ));
    this->add( "shutdown", shutdownCmd, this->tr( "terminate the applet" ));
    this->add( "sys_stressTest", stressTestCmd, this->tr( "stress test the applet" ));
    this->add( "mem_info", memInfoCmd, this->tr( "print alloc/dealloc count" ));
    this->add( "help", listCmd, this->tr( "same as cmd_list" ));

    // we are initialised
    this->setInitialised();
}

/**
 * @brief Cmd::shutdown shuts down command subsystem
 */
void Cmd::shutdown() {
    // failsafe
    if ( !this->hasInitialised())
        return;
    else
        this->setInitialised( false );

    // remove all commands
    foreach ( Command *cmdFunc, this->cmdList )
        delete cmdFunc;
    this->cmdList.clear();
}

/**
 * @brief Cmd::add adds a command
 * @param command command name
 * @param function function callback
 * @param description command description to be printed in console
 */
void Cmd::add( const QString &command, cmdCommand_t function, const QString &description ) {
    // failsafe
    if ( this->find( command ) != nullptr ) {
        Common::print( StrWarn + this->tr( "command \"%1\" already exists\n" ).arg( command ), Common::Console );
        return;
    }

    // alloc new command
    this->cmdList << new Command( command, function, description );
}

/**
 * @brief Cmd::remove removes command by name
 * @param command command name
 */
void Cmd::remove( const QString &command ) {
    Command *cmdPtr;

    cmdPtr = this->find( command );
    if ( cmdPtr != nullptr ) {
        // remove from completer
        cmdList.removeOne( cmdPtr );
        delete cmdPtr;
    }
}

/**
 * @brief Cmd::print prints text to console
 * @param args message
 */
void Cmd::print( const QStringList &args ) {
    if ( args.count() < 1 ) {
        Common::print( this->tr( "usage: con_print [message] - prints text to console\n" ), Common::Console );
        return;
    }

    // just merge everything into a single message
    Common::print( QString( "%1\n" ).arg( args.join( " " )), Common::Console );
}

/**
 * @brief Cmd::list lists all available commands
 * @param args filter
 */
void Cmd::list( const QStringList &args ) {
    // announce
    if ( !args.isEmpty()) {
        int numFiltered = 0;

        // get total filtered count
        foreach ( Command *cmdPtr, this->cmdList ) {
            if ( !args.isEmpty() && !cmdPtr->name().startsWith( args.first()))
                continue;
            numFiltered++;
        }
        if ( !numFiltered )
            Common::print( StrWarn + this->tr( "could not match any available commands\n" ), Common::Console );
        else
            Common::print( this->tr( "matched %1 of %2 available commands:\n" ).arg( numFiltered ).arg( this->cmdList.count()), Common::Console );
    } else
        Common::print( this->tr( "%1 available commands:\n" ).arg( this->cmdList.count()), Common::Console );

    foreach ( Command *cmdPtr, this->cmdList ) {
        if ( !args.isEmpty() && !cmdPtr->name().startsWith( args.first()))
            continue;

        if ( !cmdPtr->description().isEmpty() )
            Common::print( QString( " %1 - %2\n" ).arg( cmdPtr->name(), cmdPtr->description()), Common::Console );
        else
            Common::print( QString( " %1\n" ).arg( cmdPtr->name()), Common::Console );
    }
}

/**
 * @brief Cmd::listCvars lists all available console variables
 */
void Cmd::listCvars() {
    if ( !Variable::instance()->list.isEmpty())
        Common::print( QString( "%1 available console variables:" ).arg( Variable::instance()->list.count()), Common::Console );

    foreach ( VariableEntry entry, Variable::instance()->list ) {
        if ( !QString::compare( entry.key(), "system/consoleHistory" ))
            continue;

        if ( QString::compare( entry.defaultValue().toString(), entry.value().toString(), Qt::CaseInsensitive ))
            Common::print( QString( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( entry.key()).arg( entry.value().toString()).arg( entry.defaultValue().toString()), Common::Console );
        else
            Common::print( QString( "  \"%1\" is \"%2\"" ).arg( entry.key()).arg( entry.value().toString()), Common::Console );
    }
}

/**
 * @brief Cmd::cvarSet sets a new value to a console variable
 * @param args key, value
 */
void Cmd::cvarSet( const QStringList &args ) {
    if ( args.count() < 2 ) {
        Common::print( this->tr( "usage: cv_set [key] [value] - set console variable value\n" ), Common::Console );
        return;
    }

    // NOTE: this is case sensitive
    if ( Variable::instance()->contains( args.first())) {
        Common::print( QString( "no such cvar - \"%1\"" ).arg( args.first()), Common::Console );
    } else {
        VariableEntry entry;

        entry = Variable::instance()->list[args.first()];
        Common::print( QString( "setting \"%1\" to \"%2\"" ).arg( args.at( 1 )).arg( entry.key()), Common::Console );
        Variable::instance()->setValue( entry.key(), args.at( 1 ));
    }
}

/**
 * @brief Cmd::dbInfo prints database info to console
 */
void Cmd::dbInfo() {
    Common::print( QString( "events - %1, teams - %2 (%3), tasks - %4 (%5), logs - %6" )
                   .arg( Main::instance()->eventList.count())
                   .arg( Event::active()->teamList.count())
                   .arg( Main::instance()->teamList.count())
                   .arg( Event::active()->taskList.count())
                   .arg( Main::instance()->taskList.count())
                   .arg( Main::instance()->logList.count()), Common::Console );
}

/**
 * @brief Cmd::clearLogs clears ALL logs
 */
void Cmd::clearLogs() {
    QSqlQuery query;

    foreach ( Team *teamPtr, Event::active()->teamList ) {
        foreach ( Log *logPtr, teamPtr->logList ) {
            logPtr->setValue( 0 );
            query.exec( QString( "delete from logs where value=%1" ).arg( logPtr->id()));
        }
    }
}

/**
 * @brief Cmd::clearCombos clears ALL combos
 */
void Cmd::clearCombos() {
    foreach ( Team *teamPtr, Event::active()->teamList ) {
        foreach ( Log *logPtr, teamPtr->logList )
            logPtr->setComboId( -1 );
    }
}

#ifdef APPLET_DEBUG
/**
 * @brief Cmd::memInfo prints memory information to console (DEBUG)
 */
void Cmd::memInfo() {
    Common::print( QString( "meminfo: %1 allocs, %2 deallocs" )
                   .arg( Main::instance()->alloc )
                   .arg( Main::instance()->dealloc ), Common::Console );
}
#endif

/**
 * @brief Cmd::teamAdd adds a new team to current event
 * @param args name, members
 */
void Cmd::teamAdd( const QStringList &args ) {
    if ( args.count() < 2 ) {
        Common::print( this->tr( "usage: team_add [name] [members] - add a new team to the current event\n" ), Common::Console );
        return;
    }
    Team::add( args.at( 0 ), args.at( 1 ).toInt(), Event::active()->startTime(), Variable::instance()->string( "reviewerName" ), false );
}

/**
 * @brief Cmd::teamRemove removes a team from current event
 * @param args name
 */
void Cmd::teamRemove( const QStringList &args ) {
    if ( args.count() < 1 ) {
        Common::print( this->tr( "usage: team_remove [name] - remove team\n" ), Common::Console );
        return;
    }

    if ( Event::active()->teamList.indexOf( Team::forName( args.at( 0 ))) != -1 )
        Team::remove( args.at( 0 ));
}

/**
 * @brief Cmd::teamLogs removes a team from current event
 * @param args name
 */
void Cmd::teamLogs( const QStringList &args ) {
    if ( args.count() < 1 ) {
        Common::print( this->tr( "usage: teamLogs [name] - number of logs for a team\n" ), Common::Console );
        return;
    }

    Team *teamPtr = Team::forName( args.at( 0 ));
    int count = 0;
    if ( Event::active()->teamList.indexOf( teamPtr ) != -1 ) {
        foreach ( Log *logPtr, Main::instance()->logList ) {
            if ( logPtr->teamId() == teamPtr->id())
                count++;
        }
        Common::print( this->tr( "  %1 logs\n" ).arg( count ), Common::Console );
    } else
        Common::print( this->tr( "  team  \"%1\" not found\n" ).arg( args.at( 0 )), Common::Console );
}

/**
 * @brief Cmd::stressTest log imitation for debugging purposes
 * @param args stress test options (see source code)
 */
void Cmd::stressTest( const QStringList &args ) {
    if ( args.count() < 1 ) {
        Common::print( this->tr( "usage: sys_stressTest [numTeams] - stress test the applet\n" ), Common::Console );
        return;
    }

    MainWindow *gui = qobject_cast<MainWindow *>( Main::instance()->parent());
    if ( !QString::compare( args.first(), "clear" )) {
        gui->stressTest( -1 );
        return;
    }

    if ( !QString::compare( args.first(), "custom" )) {
        gui->stressTest( -2 );
        return;
    }

    if ( !QString::compare( args.first(), "combos" )) {
        foreach ( Team *teamPtr, Event::active()->teamList ) {
            int numLogs = 0;

            foreach ( Log *logPtr, teamPtr->logList ) {
                if ( logPtr->value() == 0 )
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

            foreach ( Log *logPtr, teamPtr->logList ) {
                if ( logPtr->value() == 0 )
                    continue;

                if ( comboCount == C0 )
                    break;

                // first fill the largest pool
                logList << logPtr;

                if ( comboCount == C234 && logList.count() == 4 )
                    comboCount = C23;
                else if ( comboCount == C23 && logList.count() == 3 )
                    comboCount = C2;
                else if ( comboCount == C2 && logList.count() == 2 )
                    comboCount = C0;
                else
                    continue;

                int freeHandle = Combo::getFreeHandle();
                foreach ( Log *logPtr, logList )
                    logPtr->setComboId( freeHandle );

                logList.clear();
            }
        }
        return;
    }

    if ( gui != nullptr )
        gui->stressTest( args.first().toInt());
}

/**
 * @brief Cmd::executeTokenized subdivides command string into args and executes it
 * @param command command name
 * @param args argument string list
 * @return success
 */
bool Cmd::executeTokenized( const QString &command, const QStringList &args ) {
    Command *cmdPtr;

    // find the command
    cmdPtr = this->find( command );
    if ( cmdPtr != nullptr ) {
        // execute the function
        if ( cmdPtr->hasFunction()) {
            cmdPtr->execute( args );
            return true;
        }
    }

    // find the cvar
    // NOTE: this is case sensitive
    if ( Variable::instance()->contains( command )) {
        VariableEntry entry;

        entry = Variable::instance()->list[command];
        if ( args.count() >= 1 ) {
            QStringList cvCmd;
            cvCmd.append( command );
            cvCmd << args;
            this->cvarSet( cvCmd );
        } else
            Common::print( QString( "%1\n" ).arg( entry.value().toString()), Common::Console );

        return true;
    }

    // report unknown command
    Common::print( StrWarn + this->tr( "unknown command \"%1\", prehaps try \"cmd_list\"\n" ).arg( command ), Common::Console );
    return false;
}

/**
 * @brief Cmd::find finds command entry by name
 * @param command command name
 * @return command entry
 */
Command *Cmd::find( const QString &command ) const {
    foreach ( Command *cmdPtr, this->cmdList ) {
        if ( !QString::compare( command, cmdPtr->name(), Qt::CaseInsensitive ))
            return cmdPtr;
    }
    return nullptr;
}

/**
 * @brief Command::execute executes command function callback
 * @param args arguments
 */
void Command::execute( const QStringList &args ) {
    this->m_function( args );
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
    else
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
