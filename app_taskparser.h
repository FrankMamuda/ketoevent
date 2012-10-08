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

#ifndef APP_TASKPARSER_H
#define APP_TASKPARSER_H

//
// includes
//
#include "sys_common.h"
#include "app_taskentry.h"

// team data struct
typedef struct taskData_s {
    QString name;
    App_TaskEntry::Types type;
    int points;
    int maxMulti;
    bool challenge;
} taskData_t;

//
// class:App_TaskParser
//
class App_TaskParser : public QXmlDefaultHandler {
public:
    // parse states
    enum ParseStates {
        Document = 0,
        Tasks
    };

    // constructor
    explicit App_TaskParser() {}

    // reimplemented functions
    bool startElement( const QString &, const QString &, const QString &, const QXmlAttributes & );
    bool startDocument() { this->resetState(); return true; }
    bool endElement( const QString &, const QString &, const QString & );

    // other public funcs
    void setState( ParseStates state ) { this->m_state = state; }
    void resetState() { this->m_state = Document; }
    ParseStates state() const { return this->m_state; }

private:
    ParseStates m_state;

    // temporary values
    QList <taskData_t> taskData;
};

#endif // APP_TASKPARSER_H
