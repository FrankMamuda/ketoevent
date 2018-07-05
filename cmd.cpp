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
#include "variable.h"
#include "event.h"
#include "team.h"
#include "task.h"
#include "log.h"
#include <QSqlQuery>
#include <QDebug>

/**
 * @brief Cmd::Cmd
 * @param parent
 */
Cmd::Cmd( QObject *parent ) : QObject( parent ) {
    auto listCmd = []( const QString &name, const QStringList &args ) { Cmd::instance()->list( name, args ); };

    // add common commands
    this->add( "cmd_list", +listCmd, this->tr( "list all available commands" ));
    this->add( "con_print", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->print( name, args ); }, this->tr( "print text to console" ));
    this->add( "cv_list", +[]( const QString &, const QStringList & ) { Cmd::instance()->listCvars(); }, this->tr( "list all available console variables" ));
    this->add( "cv_set", +[]( const QString &name, const QStringList &args ) { Cmd::instance()->cvarSet( name, args ); }, this->tr( "set console variable value" ));
    this->add( "db_info", +[]( const QString &, const QStringList & ) { Cmd::instance()->dbInfo(); }, this->tr( "display database information" ));
    this->add( "help", listCmd, this->tr( "same as cmd_list" ));

    // add to garbage man
    this->setObjectName( "Cmd" );
    GarbageMan::instance()->add( this );
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
    qInfo() << args.join( " " );
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
        qWarning() << this->tr( "%1 available console variables:" ).arg( Variable::instance()->list.count());

    foreach ( const QSharedPointer<Var> &entry, Variable::instance()->list ) {
        if ( entry->flags() & Var::Flag::Hidden )
            continue;

        if ( QString::compare( entry->defaultValue().toString(), entry->value().toString(), Qt::CaseInsensitive ))
            qInfo() << this->tr( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( entry->key()).arg( entry->value().toString()).arg( entry->defaultValue().toString());
        else
            qInfo() << this->tr( "  \"%1\" is \"%2\"" ).arg( entry->key()).arg( entry->value().toString());
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

    if ( !Variable::instance()->contains( args.first())) {
        qInfo() << this->tr( "no such cvar - \"%1\"" ).arg( args.first());
    } else {
        QSharedPointer<Var> entry;

        entry = Variable::instance()->list[args.first()];
        if ( entry->flags() & Var::Flag::ReadOnly ) {
            qInfo() << this->tr( "\"%1\" is read only" ).arg( entry->key());
            return;
        }

        qInfo() << this->tr( "setting \"%1\" to \"%2\"" ).arg( args.at( 1 )).arg( entry->key());
        Variable::instance()->setValue( entry->key(), args.at( 1 ));
    }
}

/**
 * @brief Cmd::dbInfo prints database info to console
 */
void Cmd::dbInfo() {
    int events = 0, teams = 0, tasks = 0, logs = 0;
    QSqlQuery query;

    // print out memory contents
    qInfo() << QString( "Current: teams - %1, tasks - %2, logs - %3" )
                   .arg( Team::instance()->count())
                   .arg( Task::instance()->count())
                   .arg( Log::instance()->count());

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

    qInfo() << QString( "Database: events - %1, teams - %2, tasks - %3, logs - %4" )
                   .arg( events ).arg( teams ).arg( tasks ).arg( logs );
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
        QSharedPointer<Var> entry;

        entry = Variable::instance()->list[name];
        if ( args.count() >= 1 ) {
            QStringList cvCmd;
            cvCmd.append( name );
            cvCmd << args;
            this->cvarSet( name, cvCmd );
        } else
            qInfo() << entry->value().toString();

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
    foreach ( const QString &string, separated ) {
        // tokenize & execute command
        if ( this->tokenize( string, command, arguments ))
            counter += this->executeTokenized( command, arguments );
    }

    if ( counter )
        return true;

    return false;
}
