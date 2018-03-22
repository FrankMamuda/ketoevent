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
#include "database.h"

/**
 * @brief The Task class
 */
class Task : public DatabaseEntry {
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( int points READ points WRITE setPoints )
    Q_PROPERTY( int multi READ multi WRITE setMulti )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( Styles style READ style WRITE setStyle )
    Q_PROPERTY( int order READ order WRITE setOrder )
    Q_PROPERTY( int eventId READ eventId WRITE setEventId )
    Q_PROPERTY( bool reindexRequired READ reindexRequired )
    Q_ENUMS( Types )
    Q_ENUMS( Styles )
    Q_CLASSINFO( "description", "Task SQL Entry" )

public:
    enum Types {
        NoType = -1,
        Check,
        Multi
    };
    enum Styles {
        NoStyle = -1,
        Regular,
        Bold,
        Italic
    };
    explicit Task( const QSqlRecord &record, const QString &table );
    QString name() const { return this->record().value( "name" ).toString(); }
    int points() const { return this->record().value( "points" ).toInt(); }
    int multi() const { if ( this->type() == Multi ) return this->record().value( "multi" ).toInt(); else return 0; }
    Types type() const { return static_cast<Types>( this->record().value( "type" ).toInt()); }
    Styles style() const { return static_cast<Styles>( this->record().value( "style" ).toInt()); }
    int order( bool sql = false ) const;
    int calculate( int logId ) const;
    int eventId() const { return this->record().value( "eventId" ).toInt(); }
    QString description() const { return this->record().value( "description" ).toString(); }
    bool reindexRequired() const { return this->m_reindex; }

    // static functions
    static Task *forId( int id );
    static Task *forName( const QString &name , bool currentEvent = true );
    static void loadTasks();
    static void add( const QString &taskName, int points, int multi, Task::Types type, Task::Styles style = Task::NoStyle , const QString &description = QString());

public slots:
    void setName( const QString &name ) { this->setValue( "name", name ); }
    void setPoints( int points = 0 ) { this->setValue( "points", points ); }
    void setMulti( int multi = 2 ) { if ( this->type() == Multi ) this->setValue( "multi", multi ); else this->setValue( "multi", 0 ); }
    void setType( Types type = Check ) { this->setValue( "type", static_cast<int>( type )); }
    void setStyle( Styles style = NoStyle ) { this->setValue( "style", static_cast<int>( style )); }
    void setOrder( int order = 0, bool direct = false );
    void setEventId( int id ) { this->setValue( "eventId", id ); }
    void setDescription( const QString &description ) { this->setValue( "description", description ); }

private:
    bool m_reindex;
    int m_order;
};
