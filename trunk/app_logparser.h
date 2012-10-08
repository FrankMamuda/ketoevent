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

#ifndef APP_LOGPARSER_H
#define APP_LOGPARSER_H

//
// includes
//
#include "sys_common.h"
#include "app_tasklog.h"

// team data struct
typedef struct teamData_s {
    QString name;
    int members;
    int hour;
    int minute;
} teamData_t;

// log data struct
typedef struct logData_s {
    QString hash;
    int value;
    App_TaskLog::Combos combo;
} logData_t;

//
// class:App_LogParser
//
class App_LogParser : public QXmlDefaultHandler {
public:
    // parse states
    enum ParseStates {
        Document = 0,
        Log
    };

    // constructor
    explicit App_LogParser() { this->setDataBaseParse( false ); }

    // reimplemented functions
    bool startElement( const QString &, const QString &, const QString &, const QXmlAttributes & );
    bool startDocument() { this->resetState(); return true; }
    bool endElement( const QString &, const QString &, const QString & );

    // other public funcs
    void setState( ParseStates state ) { this->m_state = state; }
    void resetState() { this->m_state = Document; }
    ParseStates state() const { return this->m_state; }
    QString filename() const { return this->m_filename; }
    void setFilename( const QString &filename ) { this->m_filename = filename; }
    void setDataBaseParse( bool db = true ) { this->m_db = db; }
    bool isDataBase() const { return this->m_db; }

private:
    ParseStates m_state;

    // temporary values
    teamData_t teamData;
    QString m_filename;
    bool m_db;
    QList <logData_t>logData;
};

#endif // APP_LOGPARSER_H
