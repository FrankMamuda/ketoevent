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

//
// includes
//
#include "main.h"
#include "mainwindow.h"

// latin4 chars
static unsigned int latin4Array[] = {
    224, // aa
    232, // ch
    186, // ee
    187, // gj
    239, // ii
    243, // kj
    182, // lj
    241, // nj
    242, // oo
    179, // rj
    185, // sh
    254, // uu
    190, // zh
    192, // AA
    200, // CH
    170, // EE
    171, // GJ
    207, // II
    211, // KJ
    166, // LJ
    209, // NJ
    210, // OO
    163, // RJ
    169, // SH
    222, // UU
    174  // ZH
};
static int latin4ArraySize = sizeof( latin4Array ) / sizeof( int );


// latin4 chars
static unsigned int latin4ArrayB[] = {
    257, // aa
    269, // ch
    275, // ee
    291, // gj
    299, // ii
    311, // kj
    316, // lj
    326, // nj
    333, // oo
    343, // rj
    353, // sh
    363, // uu
    382, // zh
    256, // AA
    268, // CH
    274, // EE
    290, // GJ
    298, // II
    310, // KJ
    315, // LJ
    325, // NJ
    332, // OO
    342, // RJ
    352, // SH
    362, // UU
    381  // ZH
};

// latin1 corresponding chars
static char latin1Array[] = {
    'a', // aa
    'c', // ch
    'e', // ee
    'g', // gj
    'i', // ii
    'k', // kj
    'l', // lj
    'n', // nj
    'o', // oo
    'r', // rj
    's', // sh
    'u', // uu
    'z', // zh
    'A', // AA
    'C', // CH
    'E', // EE
    'G', // GJ
    'I', // II
    'K', // KJ
    'L', // LJ
    'N', // NJ
    'O', // OO
    'R', // RJ
    'S', // SH
    'U', // UU
    'Z'  // ZH
};

/**
 * @brief Main::transliterate
 * @param path
 * @return
 */
QString Main::transliterate( const QString &path ) const {
    int y;
    QString out;

    foreach ( QChar ch, path ) {
        for ( y = 0; y < latin4ArraySize; y++ ) {
            if ( ch == QChar( latin4Array[y] ) || ch == QChar( latin4ArrayB[y] ))
                ch = latin1Array[y];
        }
        out.append( ch );
    }
    return out;
}

/**
 * @brief listToAscending
 * @param ePtr0
 * @param ePtr1
 * @return
 */
template <class T>
static bool listToAscending( T *ePtr0, T *ePtr1 ) {
    return Main::instance()->transliterate( ePtr0->name().toLower()) < Main::instance()->transliterate( ePtr1->name().toLower());
}

/**
 * @brief listByLogged
 * @param ePtr0
 * @param ePtr1
 * @return
 */
static bool listByLogged( Task *ePtr0, Task *ePtr1 ) {
    bool one = false, two = false;

    MainWindow *gui = qobject_cast<MainWindow*>( Main::instance()->parent());
    if ( gui == nullptr )
        return false;

    if ( gui->currentTeamId() == -1 )
        return false;

    Team *teamPtr = Team::forId( gui->currentTeamId());
    if ( teamPtr == nullptr )
        return false;

    foreach ( Log *logPtr, teamPtr->logList ) {
        if ( logPtr->value()) {
            if ( logPtr->taskId() == ePtr0->id())
                one = true;
            if ( logPtr->taskId() == ePtr1->id())
                two = true;

        }
    }

    // something is wrong with QString::localeAwareCompare
    if ( one == false && two == false ) {
        if ( Variable::instance()->isEnabled( "misc/sortTasks" ))
            return /*QString::localeAwareCompare( ePtr0->name(), ePtr1->name()) < 0;*/Main::instance()->transliterate( ePtr0->name().toLower()) < Main::instance()->transliterate( ePtr1->name().toLower());
        else
            return ePtr0->order() < ePtr1->order();
    }

    return one < two;
}

/**
 * @brief Main::sort
 * @param type
 */
void Main::sort( ListTypes type ) {
    switch ( type ) {
    case Tasks:
    {
        QList <Task*>regularList;
        QList <Task*>boldList;
        QList <Task*>italicList;

        foreach ( Task *taskPtr, Event::active()->taskList ) {
            if ( taskPtr->style() == Task::Regular )
                regularList << taskPtr;
        }
        qSort( regularList.begin(), regularList.end(), listToAscending<Task> );

        foreach ( Task *taskPtr, Event::active()->taskList ) {
            if ( taskPtr->style() == Task::Bold )
                boldList << taskPtr;
        }
        qSort( boldList.begin(), boldList.end(), listToAscending<Task> );

        foreach ( Task *taskPtr, Event::active()->taskList ) {
            if ( taskPtr->style() == Task::Italic )
                italicList << taskPtr;
        }
        qSort( italicList.begin(), italicList.end(), listToAscending<Task> );

        Event::active()->taskList.clear();
        Event::active()->taskList.append( regularList );
        Event::active()->taskList.append( boldList );
        Event::active()->taskList.append( italicList );

        // clean up
        regularList.clear();
        boldList.clear();
        italicList.clear();
    }
        break;

    case Teams:
        qSort( Event::active()->teamList.begin(), Event::active()->teamList.end(), listToAscending<Team> );
        break;

    case NoType:
    default:
        Common::error( StrSoftError, this->tr( "unknown list type \"%1\"\n" ).arg( static_cast<int>( type )));
        return;
    }
}

/**
 * @brief Main::taskListSorted
 * @return
 */
QList<Task*> Main::taskListSorted() {
    QList <Task*>sortedList;

    if ( Event::active() == nullptr )
        return sortedList;

    // make a local copy and sort it alphabetically or by logs (or both)
    sortedList = Event::active()->taskList;

    if ( sortedList.isEmpty())
        return sortedList;

    if ( Variable::instance()->isEnabled( "misc/sortTasks" ))
        qSort( sortedList.begin(), sortedList.end(), listToAscending<Task> );

    if ( Variable::instance()->isEnabled( "misc/hilightLogged" ))
        qSort( sortedList.begin(), sortedList.end(), listByLogged );

    // return sorted list
    return sortedList;
}

/**
 * @brief Main::teamListSorted
 * @return
 */
QList<Team*> Main::teamListSorted() {
    QList <Team*>sortedList;

    if ( Event::active() == nullptr )
        return sortedList;

    // make a local copy and sort it alphabetically or by logs (or both)
    sortedList = Event::active()->teamList;

    if ( sortedList.isEmpty())
        return sortedList;

    qSort( sortedList.begin(), sortedList.end(), listToAscending<Team> );

    // return sorted list
    return sortedList;
}
