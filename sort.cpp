/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
// sort.cpp (main.cpp is too crowded)
//

//
// includes
//
#include "main.h"

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

/*
============
transliterate
============
*/
QString Main::transliterate( const QString &path ) {
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

/*
================
listToAscending
================
*/
template <class T>
static bool listToAscending( T *ePtr0, T *ePtr1 ) {
    return m.transliterate( ePtr0->name().toLower()) < m.transliterate( ePtr1->name().toLower());
}

/*
================
sort
================
*/
void Main::sort( ListTypes type ) {
    switch ( type ) {
    case Tasks:
    {
        QList <TaskEntry*>regularList;
        QList <TaskEntry*>boldList;
        QList <TaskEntry*>italicList;

        foreach ( TaskEntry *taskPtr, this->base.taskList ) {
            if ( taskPtr->style() == TaskEntry::Regular )
                regularList << taskPtr;
        }
        qSort( regularList.begin(), regularList.end(), listToAscending<TaskEntry> );

        foreach ( TaskEntry *taskPtr, this->base.taskList ) {
            if ( taskPtr->style() == TaskEntry::Bold )
                boldList << taskPtr;
        }
        qSort( boldList.begin(), boldList.end(), listToAscending<TaskEntry> );

        foreach ( TaskEntry *taskPtr, this->base.taskList ) {
            if ( taskPtr->style() == TaskEntry::Italic )
                italicList << taskPtr;
        }
        qSort( italicList.begin(), italicList.end(), listToAscending<TaskEntry> );

        this->base.taskList.clear();
        this->base.taskList.append( regularList );
        this->base.taskList.append( boldList );
        this->base.taskList.append( italicList );


        regularList.clear();
        boldList.clear();
        italicList.clear();
        //qSort( this->base.taskList.begin(), this->base.taskList.end(), listToAscending<TaskEntry> );
    }
        break;

    case Teams:
        qSort( this->base.teamList.begin(), this->base.teamList.end(), listToAscending<TeamEntry> );
        break;

    case NoType:
    default:
        this->error( StrSoftError + this->tr( "unknown list type \"%1\"\n" ).arg( static_cast<int>( type )));
        return;
    }
}

/*
================
taskListSorted
================
*/
QList<TaskEntry*> Main::taskListSorted() {
    QList <TaskEntry*>sortedList;

    // make a local copy and sort it alphabetically
    sortedList = this->currentEvent()->taskList;
    qSort( sortedList.begin(), sortedList.end(), listToAscending<TaskEntry> );

    // return sorted list
    return sortedList;
}
