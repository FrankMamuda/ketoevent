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

#pragma once

/*
 * includes
 */
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
    Q_DISABLE_COPY_MOVE( Cmd )
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

    [[nodiscard]] bool execute( const QString & );

    /**
     * @brief contains
     * @param name
     * @return
     */
    [[nodiscard]] bool contains( const QString &name ) const { return this->functionMap.contains( name ); }

    /**
     * @brief keys
     * @return
     */
    [[nodiscard]] QStringList keys() const { return this->functionMap.keys(); }

    /**
     * @brief function
     * @param name
     * @return
     */
    [[nodiscard]][[maybe_unused]] function_t function( const QString &name ) const { if ( this->contains( name )) return this->functionMap[name]; return nullptr; }

    /**
     * @brief description
     * @param name
     * @return
     */
    [[nodiscard]] QString description( const QString &name ) const { if ( this->contains( name )) return this->descriptionMap[name]; return QString(); }

    [[nodiscard]] static bool tokenize( const QString &string, QString &command, QStringList &arguments );

    /**
     * @brief Cmd::~Cmd
     */
    ~Cmd() override { this->functionMap.clear(); this->descriptionMap.clear(); }

    /**
     * @brief instance
     * @return
     */
    static Cmd *instance() { static Cmd instance; return &instance; }

private:
    [[nodiscard]] bool executeTokenized( const QString &, const QStringList & );

    // constructor/destructor/instance
    explicit Cmd( QObject *parent = nullptr );

    QMap<QString, function_t> functionMap;
    QMap<QString, QString> descriptionMap;

public slots:
    // commands
    void list( const QString &, const QStringList & );
    void print( const QString &, const QStringList & );
    void cvarSet( const QString &, const QStringList & );
    static void dbInfo();
    void listCvars();
};
