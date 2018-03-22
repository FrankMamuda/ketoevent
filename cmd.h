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

#pragma once

//
// includes
//
#include "main.h"

// only available in debugging mode
#ifdef APPLET_DEBUG

//
// defines
//
typedef void ( *cmdCommand_t )( const QStringList &args );
Q_DECLARE_METATYPE( cmdCommand_t )

// command macros (wrappers)
#define createCommand( c, f ) static void f ## Cmd ( const QStringList &args ) { c.f( args ); }
#define createCommandPtr( c, f ) static void f ## Cmd ( const QStringList &args ) { c->f( args ); }
#define createSimpleCommand( c, f ) static void f ## Cmd ( const QStringList &args ) { Q_UNUSED( args ) c.f(); }
#define createSimpleCommandPtr( c, f ) static void f ## Cmd ( const QStringList &args ) { Q_UNUSED( args ) c->f(); }

/**
 * @brief The Command class
 */
class Command : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform console command" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( cmdCommand_t function READ function WRITE setFunction )
    Q_DISABLE_COPY( Command )

public:
    // constructor
    Command ( const QString &command, cmdCommand_t &function, const QString &description ) {
        this->setName( command );
        this->setFunction( function );
        this->setDescription( description );
    }

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    cmdCommand_t function() const { return this->m_function; }

    // other funcs
    void execute( const QStringList &args );
    bool hasFunction() const { if ( this->m_function != nullptr ) return true; return false; }

public slots:
    // property setters
    void setName( const QString &command ) { this->m_name = command; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setFunction( const cmdCommand_t &function ) { this->m_function = function; }

private:
    // properties
    cmdCommand_t m_function;
    QString m_name;
    QString m_description;
};

/**
 * @brief The Cmd class
 */
class Cmd : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialised READ hasInitialised WRITE setInitialised )
    Q_CLASSINFO( "description", "Command subsystem" )
    Q_ENUMS( ComboCount )

public:
    void add( const QString &, cmdCommand_t, const QString & = QString() );
    void remove( const QString & );
    bool execute( const QString & );
    bool hasInitialised() const { return this->m_initialised; }
    QList<Command*> cmdList;
    Command *find( const QString & ) const;
    bool tokenize( const QString &string, QString &command, QStringList &arguments );
    enum ComboCount {
        C0 = 0,
        C2,
        C23,
        C234
    };

    // constructor/destructor/instance
    ~Cmd() {}
    static Cmd *instance() { return Singleton<Cmd>::instance( Cmd::createInstance ); }

private:
    bool executeTokenized( const QString &, const QStringList & );
    bool m_initialised;

    // constructor/destructor/instance
    Cmd( QObject *parent = nullptr ) : QObject( parent ), m_initialised( false ) {}
    static Cmd *createInstance() { return new Cmd(); }

public slots:
    void init();
    void shutdown();
    void setInitialised( bool intialised = true ) { this->m_initialised = intialised; }

    // commands
    void list( const QStringList & );
    void print( const QStringList & );
    void cvarSet( const QStringList & );
    void teamAdd( const QStringList & );
    void teamRemove( const QStringList & );
    void teamLogs( const QStringList & );
    void stressTest( const QStringList & );
    void memInfo();
    void dbInfo();
    void clearLogs();
    void clearCombos();
    void listCvars();
};

#endif
