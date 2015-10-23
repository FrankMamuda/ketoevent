/*
===========================================================================
Copyright (C) 2013-2015 Avotu Briezhaudzetava

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
// includes
//
#include "main.h"
#include "cmd.h"
#include "gui_main.h"

// only available in debugging mode
#ifdef APPLET_DEBUG

//
// classes
//
class Cmd cmd;

//
// commands
//
createCommand( cmd, print )
createCommand( cmd, list )
createCommand( cmd, cvarSet )
createCommand( cmd, teamAdd )
createCommand( cmd, teamRemove )
createCommand( cmd, stressTest )
createSimpleCommand( cmd, dbInfo )
createSimpleCommand( cmd, listCvars )
createSimpleCommand( m, shutdown )
#ifdef APPLET_DEBUG
createSimpleCommand( cmd, memInfo )
#endif

/*
============
init
============
*/
void Cmd::init() {
    // add common commands
    this->add( "cmd_list", listCmd, this->tr( "list all available commands" ));
    this->add( "con_print", printCmd, this->tr( "print text to console" ));
    this->add( "cv_list", listCvarsCmd, this->tr( "list all available console variables" ));
    this->add( "cv_set", cvarSetCmd, this->tr( "set console variable value" ));
    this->add( "db_info", dbInfoCmd, this->tr( "display database information" ));
    this->add( "team_add", teamAddCmd, this->tr( "add a new team to the current event" ));
    this->add( "team_remove", teamRemoveCmd, this->tr( "remove a team" ));
    this->add( "shutdown", shutdownCmd, this->tr( "terminate the applet" ));
    this->add( "sys_stressTest", stressTestCmd, this->tr( "stress test the applet" ));
    this->add( "mem_info", memInfoCmd, this->tr( "print alloc/dealloc count" ));
    this->add( "help", listCmd, this->tr( "same as cmd_list" ));

    // we are initialized
    this->setInitialized();
}

/*
============
shutdown
============
*/
void Cmd::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;
    else
        this->setInitialized( false );

    // announce
    //m.print( StrMsg + this->tr( "shutting down command subsystem\n" ), Main::System );

    // remove all commands
    foreach ( Command *cmdFunc, this->cmdList )
        delete cmdFunc;
    this->cmdList.clear();
}

/*
============
add
============
*/
void Cmd::add( const QString &command, cmdCommand_t function, const QString &description ) {
    // failsafe
    if ( this->find( command ) != NULL ) {
        m.print( StrWarn + this->tr( "command \"%1\" already exists\n" ).arg( command ), Main::System );
        return;
    }

    // alloc new command
    this->cmdList << new Command( command, function, description );
}

/*
============
remove
============
*/
void Cmd::remove( const QString &command ) {
    Command *cmdPtr;

    cmdPtr = this->find( command );
    if ( cmdPtr != NULL ) {
        // remove from completer
        cmdList.removeOne( cmdPtr );
        delete cmdPtr;
    }
}

/*
============
print
============
*/
void Cmd::print( const QStringList &args ) {
    if ( args.count() < 1 ) {
        m.print( /*Sys::cYellow + */this->tr( "usage: con_print [message] - print text to console\n" ), Main::System );
        return;
    }

    // just merge everything into a single message
    m.print( QString( "%1\n" ).arg( args.join( " " )), Main::System );
}

/*
============
list
============
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
            m.print( StrWarn + this->tr( "could not match any available commands\n" ), Main::System );
        else
            m.print( this->tr( "matched %1 of %2 available commands:\n" ).arg( numFiltered ).arg( this->cmdList.count()), Main::System );
    } else
        m.print( this->tr( "%1 available commands:\n" ).arg( this->cmdList.count()), Main::System );

    foreach ( Command *cmdPtr, this->cmdList ) {
        if ( !args.isEmpty() && !cmdPtr->name().startsWith( args.first()))
            continue;

        if ( !cmdPtr->description().isEmpty() )
            m.print( QString( " %1 - %2\n" ).arg( cmdPtr->name(), cmdPtr->description()), Main::System );
        else
            m.print( QString( " %1\n" ).arg( cmdPtr->name()), Main::System );
    }
}

/*
============
listCvars
============
*/
void Cmd::listCvars() {
    if ( !m.cvarList.isEmpty())
        m.print( QString( "%1 available console variables:" ).arg( m.cvarList.count()), Main::System );

    foreach ( ConsoleVariable *cvarPtr, m.cvarList ) {
        if ( !QString::compare( cvarPtr->key(), "system/consoleHistory" ))
             continue;

        if ( QString::compare( cvarPtr->defaultValue().toString(), cvarPtr->value().toString()), Qt::CaseInsensitive )
            m.print( QString( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( cvarPtr->key()).arg( cvarPtr->value().toString()).arg( cvarPtr->defaultValue().toString()), Main::System );
        else
            m.print( QString( "  \"%1\" is \"%2\"" ).arg( cvarPtr->key()).arg( cvarPtr->value().toString()), Main::System );
    }
}

/*
============
cvarSet
============
*/
void Cmd::cvarSet( const QStringList &args ) {
    if ( args.count() < 2 ) {
        m.print( /*Sys::cYellow + */this->tr( "usage: cv_print [key] [value] - set console variable value\n" ), Main::System );
        return;
    }

    ConsoleVariable *cvarPtr = m.cvar( args.first());
    if ( QString::compare( cvarPtr->key(), args.first(), Qt::CaseInsensitive ))
        m.print( QString( "no such cvar - \"%1\"" ).arg( args.first()), Main::System );
    else {
        m.print( QString( "setting \"%1\" to \"%2\"" ).arg( args.at( 1 )).arg( cvarPtr->key()), Main::System );
        cvarPtr->setValue( args.at( 1 ));
    }
}

/*
============
dbInfo
============
*/
void Cmd::dbInfo() {
    m.print( QString( "events - %1, teams - %2 (%3), tasks - %4 (%5), logs - %6" )
             .arg( m.base.eventList.count())
             .arg( m.currentEvent()->teamList.count())
             .arg( m.base.teamList.count())
             .arg( m.currentEvent()->taskList.count())
             .arg( m.base.taskList.count())
             .arg( m.base.logList.count()), Main::System );
}

/*
============
memInfo
============
*/
#ifdef APPLET_DEBUG
void Cmd::memInfo() {
    m.print( QString( "meminfo: %1 allocs, %2 deallocs" )
                 .arg( m.alloc )
                 .arg( m.dealloc ), Main::System );
}
#endif

/*
============
teamAdd
============
*/
void Cmd::teamAdd( const QStringList &args ) {
    if ( args.count() < 2 ) {
        m.print( /*Sys::cYellow + */this->tr( "usage: team_add [name] [members] - add a new team to the current event\n" ), Main::System );
        return;
    }
    m.addTeam( args.at( 0 ), args.at( 1 ).toInt(), m.currentEvent()->startTime(), m.cvar( "reviewerName" )->string(), false );
}

/*
============
teamRemove
============
*/
void Cmd::teamRemove( const QStringList &args ) {
    if ( args.count() < 1 ) {
        m.print( /*Sys::cYellow + */this->tr( "usage: team_remove [name] - remove team\n" ), Main::System );
        return;
    }

    if ( m.currentEvent()->teamList.indexOf( m.teamForName( args.at( 0 ))) != -1 )
        m.removeTeam( args.at( 0 ));
}

/*
============
stressTest
============
*/
void Cmd::stressTest( const QStringList &args ) {
    if ( args.count() > 1 ) {
        m.print( this->tr( "usage: sys_stressTest [numTeams] - stress test the applet\n" ), Main::System );
        return;
    }

    Gui_Main *gui = qobject_cast<Gui_Main *>( m.parent());
    if ( !QString::compare( args.first(), "clear" )) {
        gui->stressTest( -1 );
        return;
    }

    if ( gui != NULL )
        gui->stressTest( args.first().toInt());
}

/*
============
executeTokenized
============
*/
bool Cmd::executeTokenized( const QString &command, const QStringList &args ) {
    Command *cmdPtr;
    ConsoleVariable *cvarPtr;

    // find the command
    cmdPtr = this->find( command );
    if ( cmdPtr != NULL ) {
        // execute the function
        if ( cmdPtr->hasFunction()) {
            cmdPtr->execute( args );
            return true;
        }
    }

    // find the cvar
    cvarPtr = m.cvar( command );
    if ( cvarPtr != m.defaultCvar ) {
        if ( args.count() >= 1 ) {
            QStringList cvCmd;
            cvCmd.append( command );
            cvCmd << args;
            this->cvarSet( cvCmd );
        } else
            m.print( QString( "%1\n" ).arg( cvarPtr->string()), Main::System );

        return true;
    }

    // report unknown command
    m.print( StrWarn + this->tr( "unknown command \"%1\", prehaps try \"cmd_list\"\n" ).arg( command ), Main::System );
    return false;
}

/*
============
find
============
*/
Command *Cmd::find( const QString &command ) const {
    foreach ( Command *cmdPtr, this->cmdList ) {
        if ( !QString::compare( command, cmdPtr->name(), Qt::CaseInsensitive ))
            return cmdPtr;
    }
    return NULL;
}

/*
============
execute
============
*/
void Command::execute( const QStringList &args ) {
    this->m_function( args );
}

/*
============
tokenize
============
*/
bool Cmd::tokenize( const QString &string, QString &command, QStringList &arguments ) {
    int pos = 0, len;
    QString capture;
    QRegExp rx;

    // make sure input is blank
    command.clear();
    arguments.clear();

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
        arguments.append( capture );
        pos += len;
    }

    if ( command.isEmpty())
        return false;
    else
        return true;
}

/*
============
execute
============
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
