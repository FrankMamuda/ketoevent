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
#include <QMap>
#include <QObject>

//
// defines
//
typedef void ( *function_t )( const QString &name, const QStringList &args );
Q_DECLARE_METATYPE( function_t )

/**
 * @brief The Cmd class
 */
class Cmd final : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( Cmd )
    Q_CLASSINFO( "description", "Command subsystem" )

public:
    void add( const QString &, function_t, const QString & = QString());

    /**
     * @brief remove
     */
    void remove( const QString &name ) {
        this->functionMap.remove( name );
        this->descriptionMap.remove( name );
    }

    bool execute( const QString & );

    /**
     * @brief contains
     * @param name
     * @return
     */
    bool contains( const QString &name ) const { return this->functionMap.contains( name ); }

    /**
     * @brief keys
     * @return
     */
    QStringList keys() const { return this->functionMap.keys(); }

    /**
     * @brief function
     * @param name
     * @return
     */
    function_t function( const QString &name ) const { if ( this->contains( name )) return this->functionMap[name]; return nullptr; }

    /**
     * @brief description
     * @param name
     * @return
     */
    QString description( const QString &name ) const { if ( this->contains( name )) return this->descriptionMap[name]; return QString(); }

    bool tokenize( const QString &string, QString &command, QStringList &arguments );

    /**
     * @brief Cmd::~Cmd
     */
    virtual ~Cmd() { this->functionMap.clear(); this->descriptionMap.clear(); }

    /**
     * @brief instance
     * @return
     */
    static Cmd *instance() { static Cmd *instance( new Cmd()); /*GarbageMan::instance()->add( instance );*/ return instance; }

private:
    bool executeTokenized( const QString &, const QStringList & );

    // constructor/destructor/instance
    explicit Cmd( QObject *parent = nullptr );
    static Cmd *createInstance() { return new Cmd(); }

    QMap<QString, function_t> functionMap;
    QMap<QString, QString> descriptionMap;

public slots:
    // commands
    void list( const QString &, const QStringList & );
    void print( const QString &, const QStringList & );
    void cvarSet( const QString &, const QStringList & );
    void dbInfo();
    void listCvars();
};
