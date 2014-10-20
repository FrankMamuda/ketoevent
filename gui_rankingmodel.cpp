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
// includes
//
#include "gui_rankingmodel.h"
#include <QFont>
#include <QColor>

/*
================
data
================
*/
QVariant Gui_RankingModel::data( const QModelIndex &index, int role ) const {
    int points = 0;

    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= m.currentEvent()->teamList.count() || index.row() < 0 )
        return QVariant();

    if ( role == Qt::FontRole ) {
        if ( index.column() == TeamName || index.column() == Points ) {
            QFont font;
            font.setBold( true );
            return font;
        }
    }

    if ( role == Qt::ForegroundRole ) {
        TeamEntry *teamPtr = m.currentEvent()->teamList.at( index.row());
        if ( teamPtr->disqualified())
            return QColor( Qt::red );
    }

    if ( role == Qt::DisplayRole ) {
        TeamEntry *teamPtr = m.currentEvent()->teamList.at( index.row());

        switch ( index.column()) {
        case TeamName:
            return teamPtr->name();

        case Tasks:
            return teamPtr->logList.count();

        case Combos:
            return teamPtr->combos();

        case Total:
            return teamPtr->total();

        case Time:
            return teamPtr->timeOnTrack();

        case Reviewer:
            return teamPtr->reviewer();

        case Penalty:
            return teamPtr->penalty();

            //
            // VERY IMPORTANT: penalty points are only substracted here, for display purposes
            //
        case Points:
        {
            TeamEntry *teamPtr = m.currentEvent()->teamList.at( index.row());

            if ( teamPtr->disqualified())
                return -1;

            points = teamPtr->points() - teamPtr->penalty();

            if ( points <= 0 )
                points = 0;

            return points;
        }
            break;

        default:
            return QVariant();
        }
    } else if ( role == Qt::TextColorRole ) {
        if ( index.column() == Penalty ) {
            if ( this->data( index, Qt::DisplayRole ).toInt() > 0 )
                return QColor( Qt::red );
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

        case Combos:
            return this->tr( "Combos" );

        case Total:
            return this->tr( "Combined\ntasks" );

        case Time:
            return this->tr( "Time\n(min)" );

        case Penalty:
            return this->tr( "Penalty\npoints" );

        case Reviewer:
            return this->tr( "Reviewer" );

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

    return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}
