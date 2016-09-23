/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

#ifndef DATABASE_H
#define DATABASE_H

//
// includes
//
#include <QObject>
#include "databaseentry.h"
#include "team.h"
#include "log.h"
#include "event.h"
#include "task.h"

/**
 * @brief The tableField struct
 */
typedef struct tableField_s {
    const char *name;
    const char *type;
} tableField_t;

/**
 * @brief The table struct
 */
typedef struct table_s {
    const char *name;
    const tableField_t *fields;
    const unsigned int numFields;
} table_t;

//
// API9
//
namespace API {
// tasks
const static tableField_t taskFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 128 )" },
    { "points", "integer" },
    { "multi", "integer" },
    { "style", "integer" },
    { "type", "integer" },
    { "parent", "integer" },
    { "eventId", "integer" },
    { "description", "varchar( 512 )" }
};

// teams
const static tableField_t teamFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 64 )" },
    { "members", "integer" },
    { "finishTime", "varchar( 5 )" },
    { "lock", "integer" },
    { "reviewer", "varchar( 64 )" },
    { "eventId", "integer" }
};

// logs
const static tableField_t logFields[] = {
    { "id", "integer primary key" },
    { "value", "integer" },
    { "taskId", "integer" },
    { "teamId", "integer" },
    { "comboId", "integer" }
};

// events
const static tableField_t eventFields[] = {
    { "id", " integer primary key" },
    { "api", " integer" },
    { "name", "varchar( 64 )" },
    { "minMembers", "integer" },
    { "maxMembers", "integer" },
    { "startTime", "varchar( 5 )" },
    { "finishTime", "varchar( 5 )" },
    { "finalTime", "varchar( 5 )" },
    { "penalty", "integer" },
    { "comboOfTwo", "integer" },
    { "comboOfThree", "integer" },
    { "comboOfFourAndMore", "integer" },
    { "lock", "integer" }
};

// tables
const static table_t tables[] = {
    { "teams",  teamFields, sizeof( teamFields ) / sizeof( tableField_t ) },
    { "tasks",  taskFields, sizeof( taskFields ) / sizeof( tableField_t ) },
    { "logs",   logFields, sizeof( logFields ) / sizeof( tableField_t ) },
    { "events", eventFields, sizeof( eventFields ) / sizeof( tableField_t ) }
};
const unsigned int numTables = sizeof( tables ) / sizeof( table_t );
}

//
// API0 (2013 event)
//
namespace API0 {
// tasks
const static tableField_t taskFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 128 )" },
    { "points", "integer" },
    { "multi", "integer" },
    { "style", "integer" },
    { "type", "integer" },
    { "parent", "integer" },
};

// teams
const static tableField_t teamFields[] = {
    { "id", "integer primary key" },
    { "name", "varchar( 64 )" },
    { "members", "integer" },
    { "finish", "varchar( 5 )" },
};

// logs
const static tableField_t logFields[] = {
    { "id", "integer primary key" },
    { "value", "integer" },
    { "combo", "integer" },
    { "taskId", "integer" },
    { "teamId", "integer" }
};

// tables
const static table_t tables[] = {
    { "teams",  teamFields, sizeof( teamFields ) / sizeof( tableField_t ) },
    { "tasks",  taskFields, sizeof( taskFields ) / sizeof( tableField_t ) },
    { "logs",   logFields, sizeof( logFields ) / sizeof( tableField_t ) },
};
const unsigned int numTables = sizeof( tables ) / sizeof( table_t );
}

/**
 * @brief The Database class
 */
class Database : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Database" )
    Q_ENUMS( IdTypes )
    Q_ENUMS( Import )

public:
    // id types
    enum IdTypes {
        NoId = -1,
        TeamId,
        LogId,
        ComboId,
        TaskId
    };

    enum Import {
        LogImport = 0,
        TaskImport
    };

    static int highestId( IdTypes type );
    static QString stringToHash( const QString &input );
    static QStringList generateSchemas( const QString &prefix = QString::null );
    static void attach( const QString &path, Import = LogImport );
    static void makePath( const QString &path = QString::null );
    static bool load();
    static void unload();
    static void reload( const QString &path ) { Database::unload(); Database::makePath( path ); Database::load(); }
    static void removeOrphanedLogs();
    static bool createStructure( const QString &prefix = "" );
    static bool createEmptyTable( const QString &prefix = QString::null );
    static void reindexTasks();
    static void writeBackup();
};

#endif // DATABASE_H
