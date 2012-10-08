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
#include "gui_rankingmodel.h"
#include "sys_common.h"
#include <QFont>

/*
================
data
================
*/
QVariant Gui_RankingModel::data( const QModelIndex &index, int role ) const {
    int points = 0;

    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= m.teamList.count() || index.row() < 0 )
        return QVariant();

    if ( role == Qt::FontRole ) {
        if ( index.column() == TeamName || index.column() == Points ) {
            QFont font;
            font.setBold( true );
            return font;
        }
    }

    if ( role == Qt::DisplayRole ) {
        App_TeamEntry *entryPtr = m.teamList.at( index.row());

        switch ( index.column()) {
        case TeamName:
            return entryPtr->name();

        case Tasks:
            return entryPtr->logList.count();

        case Challenges:
            return entryPtr->challenges();

        case Combos:
            return entryPtr->combos();

        case Grade:
            return entryPtr->grade();

        case Time:
            return entryPtr->timeOnTrack();

        case Penalty:
            return entryPtr->penalty();

            //
            // VERY IMPORTANT: penalty points are only substracted here, for display purposes
            //
        case Points:
            points = entryPtr->points() - entryPtr->penalty();

            if ( points <= 0 )
                points = 0;

            return points;

        default:
            return QVariant();
        }
    } else if ( role == Qt::TextColorRole ) {
        if ( index.column() == Penalty ) {
            if ( this->data( index, Qt::DisplayRole ).toInt() > 0 )
                return Qt::red;
        }
    } else if ( role == Qt::TextAlignmentRole )
        return Qt::AlignCenter;

    return QVariant();
}

/*
================
headerData
================
*/
QVariant Gui_RankingModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( role != Qt::DisplayRole )
        return QVariant();

    if ( orientation == Qt::Horizontal ) {
        switch ( section ) {
        case TeamName:
            return this->tr( "Team name" );

        case Tasks:
            return this->tr( "Tasks" );

        case Challenges:
            return this->tr( "Challenges" );

        case Combos:
            return this->tr( "Combos" );

        case Grade:
            return this->tr( "Grade" );

        case Time:
            return this->tr( "Time\n(min)" );

        case Penalty:
            return this->tr( "Penalty\npoints" );

        case Points:
            return this->tr( "Total\npoints" );

        default:
            return QVariant();
        }
    }
    return QVariant();
}

/*
================
flags
================
*/
Qt::ItemFlags Gui_RankingModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
}
