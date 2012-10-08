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

//
// includes
//
#include <QtGui/QApplication>
#include "gui_main.h"
#include "app_logparser.h"
#include "app_taskparser.h"
#include "app_main.h"
#include "sys_filesystem.h"
#include "sys_cvar.h"
#include <QMessageBox>

//
// classes
//
class App_Main m;

//
// cvars
//
pCvar *k_minimumTeamMembers;
pCvar *k_maximumTeamMembers;
pCvar *k_eventStartTime;
pCvar *k_eventFinishTime;
pCvar *k_eventFinalTime;
pCvar *k_singleComboPoints;
pCvar *k_doubleComboPoints;
pCvar *k_tripleComboPoints;
pCvar *k_defaultTaskList;
pCvar *k_penaltyMultiplier;
pCvar *k_autoSave;
pCvar *k_drunkMode;

/*
================
taskForHash

  sounds like a pun
================
*/
App_TaskEntry *App_Main::taskForHash( const QString &hash ) {
    // find it
    foreach ( App_TaskEntry *task, this->taskList ) {
        if ( !QString::compare( hash, task->hash()))
            return task;
    }
    return NULL;
}

/*
================
shutdown
================
*/
void App_Main::shutdown() {
    // shut down subsystems
    cv.saveConfig( Cvar::DefaultConfigFile );
    fs.shutdown();
    cv.shutdown();

    // close app completely
    QApplication::quit();
}

/*
================
parseXML
================
*/
template <class T>
bool App_Main::parseXML( const QString &filename, const QString &buffer, T *handlerPtr ) {
    QXmlSimpleReader xmlParser;
    QXmlInputSource *sourcePtr;

    // check for buffer
    if ( buffer.isEmpty()) {
        com.error( StrSoftError + this->tr( "called with empty buffer (\"%1\")\n" ).arg( filename ));
        return false;
    }

    // check for handler
    if ( handlerPtr == NULL ) {
        com.error( StrSoftError + this->tr( "called with NULL XML handler (\"%1\")\n" ).arg( filename ));;
        return false;
    }

    // set up parser
    sourcePtr = new QXmlInputSource();
    sourcePtr->setData( buffer );
    xmlParser.setContentHandler( handlerPtr );

    // read xml and report
    if ( !xmlParser.parse( sourcePtr )) {
        com.error( StrSoftError + this->tr( "parse failed (\"%1\")\n" ).arg( filename ));
        return false;
    }

    // delete the source
    delete sourcePtr;
    return true;
}

/*
================
loadTeams
================
*/
void App_Main::loadTeams() {
    QStringList teamLogs;
    QRegExp rx;
    App_LogParser *handlerPtr;

    // list all xml files in log dir
    rx.setPattern( "*.xml" );
    rx.setPatternSyntax( QRegExp::Wildcard );
    teamLogs = fs.list( "logs/", rx, Sys_Filesystem::ListFiles );

    // set up xml handler
    handlerPtr = new App_LogParser();

    // read all team logs
    foreach ( QString filename, teamLogs ) {
        QString buffer = fs.readTextFile( filename, "UTF-8" );
        handlerPtr->setFilename( filename );
        this->parseXML( filename, buffer, handlerPtr );
        buffer.clear();
    }
    this->sort( App_Main::Teams );

    // get rid of handler
    delete handlerPtr;
}

/*
================
listToAscending
================
*/
template <class T>
bool listToAscending( T *ePtr0, T *ePtr1 ) {
    return fs.transliteratePath( ePtr0->name().toLower()) < fs.transliteratePath( ePtr1->name().toLower());
}

/*
================
loadTasks
================
*/
void App_Main::loadTasks() {
    App_TaskParser *handlerPtr;
    QString buffer;

    // set up xml handler
    handlerPtr = new App_TaskParser();

    // read all team logs
    buffer = fs.readTextFile( k_defaultTaskList->string(), "UTF-8", Sys_Filesystem::SkipInternal );
    if ( !this->parseXML( k_defaultTaskList->string(), buffer, handlerPtr )) {
        com.print( StrMsg + this->tr( "reading internal task list\n" ));

        // read internal file if we fail
        buffer = fs.readTextFile( ":/tasks.xml", "UTF-8" );
        if ( !this->parseXML( ":/tasks.xml", buffer, handlerPtr ))
            com.error( StrFatalError + this->tr( "could not load internal task list\n" ));
    }
    this->sort( App_Main::Tasks );

    // get rid of handler
    delete handlerPtr;
}

/*
================
addTeam
================
*/
bool App_Main::addTeam( App_TeamEntry *teamPtr, bool sort, bool allowReplace ) {
    int y = 0;

    if ( teamPtr == NULL )
        return false;

    // check for duplicates
    foreach ( App_TeamEntry *entryPtr, this->teamList ) {
        if ( !QString::compare( entryPtr->name(), teamPtr->name())) {
            if ( allowReplace ) {
                this->teamList.replace( y, teamPtr );
                return true;
            } else {
                com.error( StrSoftError + this->tr( "team \"%1\" already exists\n" ).arg( teamPtr->name()));
                return false;
            }
        }
        y++;
    }

    // add to list
    this->teamList << teamPtr;

    // sort alphabetically
    if ( sort )
        this->sort( App_Main::Teams );

    // make sure we update list in gui
    this->updateTeamList();
    return true;
}

/*
================
updateTeamList
================
*/
void App_Main::updateTeamList() {
    this->sort( App_Main::Teams );
    emit this->teamListChanged();
}

/*
================
removeTeam

  must move XML somewhere
================
*/
void App_Main::removeTeam( App_TeamEntry *teamPtr ) {
    if ( teamPtr == NULL )
        return;

    // remove from list (set it to disabled and ignore from now on)
    teamPtr->setDisabled( true );
    teamPtr->save();
    this->teamList.removeOne( teamPtr );
    this->updateTeamList();
}

/*
================
addTask
================
*/
bool App_Main::addTask( App_TaskEntry *taskPtr, bool sort ) {
    if ( taskPtr == NULL )
        return false;

    // check for duplicates
    foreach ( App_TaskEntry *entryPtr, this->taskList ) {
        if ( !QString::compare( entryPtr->name(), taskPtr->name())) {
            com.error( StrSoftError + this->tr( "task \"%1\" already exists\n" ).arg( taskPtr->name()));
            return false;
        }
    }
    // add to list
    this->taskList << taskPtr;

    // sort alphabetically
    if ( sort )
        this->sort( App_Main::Tasks );

    // make sure we update list in gui
    this->updateTaskList();
    return true;
}

/*
================
updateTaskList
================
*/
void App_Main::updateTaskList() {
    QString buffer;
    fileHandle_t out;

    // send updates
    this->sort( App_Main::Tasks );
    emit this->taskListChanged();

    // save just in case
    if ( k_defaultTaskList->string().isEmpty()) {
        com.error( StrSoftError + this->tr( "cannot save tasks, no filename set - resetting\n" ));
        k_defaultTaskList->reset();
    }

    // add extension
    if ( !k_defaultTaskList->string().endsWith( ".xml" ))
        k_defaultTaskList->setString( k_defaultTaskList->string() + ".xml" );

    // generate buffer
    buffer.append( "<tasks>\n" );
    foreach ( App_TaskEntry *entryPtr, this->taskList )
        buffer.append( entryPtr->generateXMLTag());
    buffer.append( "</tasks>\n" );

    // open file in read mode
    fs.open( pFile::Write, k_defaultTaskList->string(), out );
    fs.print( out, buffer );
    fs.close( out );
}

/*
================
removeTask
================
*/
void App_Main::removeTask( App_TaskEntry *taskPtr ) {
    if ( taskPtr == NULL )
        return;

    // add to list
    this->taskList.removeOne( taskPtr );
    this->updateTaskList();
}

/*
================
sort
================
*/
void App_Main::sort( ListTypes type ) {
    switch ( type ) {
    case Tasks:
        qSort( this->taskList.begin(), this->taskList.end(), listToAscending<App_TaskEntry> );
        break;

    case Teams:
        qSort( this->teamList.begin(), this->teamList.end(), listToAscending<App_TeamEntry> );
        break;

    case NoType:
    default:
        com.error( StrSoftError + this->tr( "unknown list type \"%1\"\n" ).arg( static_cast<int>( type )));
        return;
    }
}

/*
================
findTeamByName
================
*/
App_TeamEntry *App_Main::findTeamByName( const QString &name ) {
    foreach ( App_TeamEntry *entryPtr, this->teamList ) {
        if ( !QString::compare( entryPtr->name(), name ))
            return entryPtr;
    }
    return NULL;
}

/*
================
findTaskByName
================
*/
App_TaskEntry *App_Main::findTaskByName( const QString &name ) {
    foreach ( App_TaskEntry *entryPtr, this->taskList ) {
        if ( !QString::compare( entryPtr->name(), name ))
            return entryPtr;
    }
    return NULL;
}

/*
================
entry point
================
*/
int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    // i18n
    QTranslator translator;
    QString locale;
#ifndef FORCE_LATVIAN
    locale = QLocale::system().name();
#else
    locale = "lv_LV";
#endif
    translator.load( QString( ":/i18n/KetoEvent_%1" ).arg( locale ));
    a.installTranslator( &translator );

    // init subsystems
    cv.init();
    fs.init();
    cv.parseConfig( Cvar::DefaultConfigFile );

    // init cvars
    k_minimumTeamMembers = cv.create( "k_minimumTeamMembers", "2", pCvar::Archive );
    k_maximumTeamMembers = cv.create( "k_maximumTeamMembers", "4", pCvar::Archive );
    k_eventStartTime = cv.create( "k_eventStartTime", "12:00", pCvar::Archive );
    k_eventFinishTime = cv.create( "k_eventFinishTime", "17:30", pCvar::Archive );
    k_eventFinalTime = cv.create( "k_eventFinalTime", "18:00", pCvar::Archive );
    k_singleComboPoints = cv.create( "k_singleComboPoints", "1", pCvar::Archive );
    k_doubleComboPoints = cv.create( "k_doubleComboPoints", "3", pCvar::Archive );
    k_tripleComboPoints = cv.create( "k_tripleComboPoints", "5", pCvar::Archive );
    k_defaultTaskList = cv.create( "k_defaultTaskList", "tasks.xml", pCvar::Archive );
    k_penaltyMultiplier = cv.create( "k_penaltyMultiplier", "5", pCvar::Archive );
    k_autoSave = cv.create( "k_autoSave", "1", pCvar::Archive );
    k_drunkMode = cv.create( "k_drunkMode", "0", pCvar::Archive );

    // load tasks & teams
    m.loadTasks();
    m.loadTeams();

    // init gui
    Gui_Main mainWindow;
    mainWindow.show();

    return a.exec();
}
