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

#ifndef APP_TEAMENTRY_H
#define APP_TEAMENTRY_H

//
// includes
//
#include "sys_shared.h"
#include "sys_common.h"
#include "app_tasklog.h"

//
// class:App_TeamEntry
//
class App_TeamEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( int members READ members WRITE setMembers )
    Q_PROPERTY( QTime startTime READ startTime WRITE setStartTime )
    Q_PROPERTY( QTime finishTime READ finishTime WRITE setFinishTime )
    Q_PROPERTY( QString filename READ filename WRITE setFilename )
    Q_PROPERTY( bool disabled READ disabled WRITE setDisabled )

public:
    explicit App_TeamEntry( const QString &name, int members = 2, const QTime &startTime = QTime(), const QTime &finishTime = QTime());
    ~App_TeamEntry() { this->logList.clear(); }
    QString name() const { return this->m_name; }
    int members() const { return this->m_members; }
    QTime startTime() const { return this->m_startTime; }
    QTime finishTime() const { return this->m_finishTime; }
    QList <App_TaskLog*> logList;
    QString filename() const { return this->m_filename; }
    QString generateXMLTag();
    QString generateXMLLog();
    bool filenameSet() const { return !this->filename().isEmpty(); }
    bool disabled() const { return this->m_disabled; }

    // for rankings
    int points() const;
    int challenges() const;
    int combos() const;
    int penalty() const;
    float grade() const;
    int timeOnTrack() const;

public slots:
    void setStartTime( const QTime &time );
    void setFinishTime( const QTime &time );
    void setName( const QString &name ) { this->m_name = name; }
    void setMembers( int members );
    void setFilename( const QString &filename ) { this->m_filename = filename; }
    void save();
    void setDisabled( bool disabled = true ) { this->m_disabled = disabled; }

private:
    QString m_name;
    int m_members;
    QTime m_startTime;
    QTime m_finishTime;
    QString m_filename;
    bool m_disabled;
};

#endif // APP_TEAMENTRY_H
