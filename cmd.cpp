/*
 * Copyright (C) 2013-2019 Factory #12
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
#include "main.h"
#include "cmd.h"
#include "mainwindow.h"
#include "variable.h"
#include "event.h"
#include "team.h"
#include "task.h"
#include "log.h"
#include "database.h"
#include <QSqlQuery>
#include <QDebug>

// fixes msvc compile issues
#ifdef Q_CC_MSVC
static constexpr const char *testEventName( "Test" );
#endif

/**
 * @brief Cmd::Cmd
 * @param parent
 */
Cmd::Cmd( QObject *parent ) : QObject( parent ) {
    auto listCmd = []( const QString &name, const QStringList &args ) { Cmd::instance().list( name, args ); };
    // disable for now on MSVC
#ifndef Q_CC_MSVC
    constexpr const char *testEventName( "Test" );
#endif

    // add common commands
    this->add( "cmd_list", static_cast<void(*)( const QString &, const QStringList & )>( listCmd ), this->tr( "list all available commands" ));
    this->add( "con_print", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &name, const QStringList &args ) { Cmd::instance().print( name, args ); } ), this->tr( "print text to console" ));
    this->add( "cv_list", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &, const QStringList & ) { Cmd::instance().listCvars(); } ), this->tr( "list all available console variables" ));
    this->add( "cv_set", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &name, const QStringList &args ) { Cmd::instance().cvarSet( name, args ); } ), this->tr( "set console variable value" ));
    this->add( "db_info", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &, const QStringList & ) { Cmd::instance().dbInfo(); } ), this->tr( "display database information" ));
    this->add( "help", static_cast<void(*)( const QString &, const QStringList & )>( listCmd ), this->tr( "same as cmd_list" ));

    // setting up test environment
    this->add( "test_setup", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &, const QStringList &args ) {
        int y, k;
        int numTeams = 5, numTasks = 10;

        // custom args
        if ( args.count() == 2 ) {
            numTeams = args.at( 0 ).toInt();
            numTasks = args.at( 1 ).toInt();
        }

        // add event
        const Row event = Event::instance().add( testEventName );
        if ( event == Row::Invalid )
            return;

        // set current event, so that new teams are added to it
        MainWindow::instance().setCurrentEvent( event );

        // add tasks
        for ( y = 0; y < numTasks; y++ )
            Task::instance().add( "Task " + QString::number( y ), rand() % 20 + 1, rand() % 5 + 1, static_cast<Task::Types>( rand() % 2 ), static_cast<Task::Styles>( rand() % 3 ), "" );

        // add teams
        for ( y = 0; y < numTeams; y++ ) {
            Row team = Team::instance().add( "Team " + QString::number( y ), rand() % EventTable::DefaultMaxMembers + 1, QTime(), "Test" );
            if ( team == Row::Invalid )
                continue;

            // add logs
            for ( k = 0; k < numTasks; k++ ) {
                const Row row = Task::instance().row( k );
                if ( row == Row::Invalid )
                    continue;

                const int value = rand() % (( Task::instance().type( row ) == Task::Types::Check ) ? 2 : Task::instance().multi( row ) + 1 );
                if ( !value )
                    continue;

                Log::instance().add( Task::instance().id( row ), Team::instance().id( team ), Task::instance().type( row ) == Task::Types::Check ? true : value );
            }
        }

        // relock ui elements if required
        MainWindow::instance().setLock();
    } ), this->tr( "add a demo event with teams, tasks and logs" ));

    // clear test environment
    this->add( "test_clear", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &, const QStringList & ) {
        int y;

        // remove all events
        for ( y = 0; y < Event::instance().count(); y++ ) {
            const Row row = Event::instance().row( y );
            if ( row == Row::Invalid )
                continue;


            if ( !QString::compare( Event::instance().title( row ), testEventName )) {
                 Event::instance().remove( row );
                 break;
            }
        }

        // clean up orphans
        Database::instance().removeOrphanedEntries();
        MainWindow::instance().setLock();
    } ), this->tr( "clear demo event" ));

    // obliterate all entries within database
    this->add( "test_delete_db", static_cast<void(*)( const QString &, const QStringList & )>( []( const QString &, const QStringList & ) {
        QSqlQuery query;
        query.exec( QString( "delete from %1" ).arg( Event::instance().tableName()));
        query.exec( QString( "delete from %1" ).arg( Log::instance().tableName()));
        query.exec( QString( "delete from %1" ).arg( Team::instance().tableName()));
        query.exec( QString( "delete from %1" ).arg( Task::instance().tableName()));

        Event::instance().select();
        Log::instance().select();
        Team::instance().select();
        Task::instance().select();
        MainWindow::instance().setLock();
    } ), this->tr( "obliterate all entries within database " ));

    // add to garbage man
    this->setObjectName( "Cmd" );
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
    const QStringList keys( this->functionMap.keys());

    // announce
    if ( !args.isEmpty()) {
        int numFiltered = 0;

        // get total filtered count
        for ( const QString &name : keys ) {
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

    for ( const QString &name : keys ) {
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
    if ( !qAsConst( Variable::instance().list ).isEmpty())
        qWarning() << this->tr( "%1 available console variables:" ).arg( qAsConst( Variable::instance().list).count());

    for ( const QSharedPointer<Var> &entry : qAsConst( Variable::instance().list )) {
        if ( entry->flags() & Var::Flag::Hidden )
            continue;

        if ( QString::compare( entry->defaultValue().toString(), entry->value().toString(), Qt::CaseInsensitive ))
            qInfo() << this->tr( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( entry->key(), entry->value().toString(), entry->defaultValue().toString());
        else
            qInfo() << this->tr( "  \"%1\" is \"%2\"" ).arg( entry->key(), entry->value().toString());
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

    if ( !Variable::instance().contains( args.first())) {
        qInfo() << this->tr( "no such cvar - \"%1\"" ).arg( args.first());
    } else {
        QSharedPointer<Var> entry;

        entry = Variable::instance().list[args.first()];
        if ( entry->flags() & Var::Flag::ReadOnly ) {
            qInfo() << this->tr( "\"%1\" is read only" ).arg( entry->key());
            return;
        }

        qInfo() << this->tr( "setting \"%1\" to \"%2\"" ).arg( args.at( 1 ), entry->key());
        Variable::setValue( entry->key(), args.at( 1 ));
    }
}

/**
 * @brief Cmd::dbInfo prints database info to console
 */
void Cmd::dbInfo() {
    int events = 0, teams = 0, tasks = 0, logs = 0;
    QSqlQuery query;

    // get memory contents
    qInfo() << QString( "Current: teams - %1, tasks - %2, logs - %3" )
                   .arg( Team::instance().count())
                   .arg( Task::instance().count())
                   .arg( Log::instance().count());

    // get actual databse contents
    query = QSqlQuery( QString( "select count(*) from %1" ).arg( Event::instance().tableName()));
    if ( query.next())
        events = query.value( 0 ).toInt();
    query = QSqlQuery( QString( "select count(*) from %1" ).arg( Team::instance().tableName()));
    if ( query.next())
        teams = query.value( 0 ).toInt();
    query = QSqlQuery( QString( "select count(*) from %1" ).arg( Task::instance().tableName()));
    if ( query.next())
        tasks = query.value( 0 ).toInt();
    query = QSqlQuery( QString( "select count(*) from %1" ).arg( Log::instance().tableName()));
    if ( query.next())
        logs = query.value( 0 ).toInt();

    // print out
    qInfo() << QString( "Database: events - %1 (%2), teams - %3(%4), tasks - %5(%6), logs - %7(%8)" )
                   .arg( events )
               .arg( Event::instance().count())
               .arg( teams )
               .arg( Team::instance().count())
               .arg( tasks )
               .arg( Task::instance().count())
               .arg( logs )
               .arg( Log::instance().count());
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
    if ( Variable::instance().contains( name )) {
        QSharedPointer<Var> entry;

        entry = Variable::instance().list[name];
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

    return !command.isEmpty();
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
    for ( const QString &string : qAsConst( separated )) {
        // tokenize & execute command
        if ( this->tokenize( string, command, arguments ))
            counter += this->executeTokenized( command, arguments );
    }

    return counter != 0;
}
