/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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

#ifndef APP_TASKENTRY_H
#define APP_TASKENTRY_H

//
// includes
//
#include "sys_shared.h"

//
// class:App_TaskEntry
//
class App_TaskEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int points READ points WRITE setPoints )
    Q_PROPERTY( int maxMulti READ maxMulti WRITE setMaxMulti )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( bool challenge READ isChallenge WRITE setChallenge )
    Q_ENUMS( Types )

public:
    enum Types {
        NoType = -1,
        Check,
        Multi,
        Special
    };
    explicit App_TaskEntry( const QString &name, Types = Check, int points = 0, int multi = 0, bool challenge = false );
    QString name() const { return this->m_name; }
    int points() const { return this->m_points; }
    int maxMulti() const { return this->m_multi; }
    QString hash() const { return this->m_hash; }
    Types type() const { return this->m_type; }
    bool isChallenge() const { if ( this->type() == Special ) return false; else return this->m_challenge; }
    QString generateXMLTag();

public slots:
    int calculate( QObject * );
    void setName( const QString & );
    void setPoints( int points ) { if ( points < 0 ) this->m_points = 0; this->m_points = points; }
    void setMaxMulti( int multi ) { this->m_multi = multi; }
    void setType( Types type ) { this->m_type = type; }
    void setChallenge( bool challenge ) { this->m_challenge = challenge; }

private:
    QString m_name;
    int m_points;
    int m_multi;
    Types m_type;
    QString m_hash;
    bool m_challenge;
};

#endif // APP_TASKENTRY_H
