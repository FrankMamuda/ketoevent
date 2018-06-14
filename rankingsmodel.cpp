/*
 * Copyright (C) 2018 Factory #12
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
#include "rankingsmodel.h"
#include "rankings.h"

/**
 * @brief RankingsModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant RankingsModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole ) {
            if ( section == 0 )
                return "Team\ntitle";

            if ( section == 1 )
                return "Completed";

            if ( section == 2 )
                return "Combos";

            if ( section == 3 )
                return "Combined";

            if ( section == 4 )
                return "Penalty";

            if ( section == 5 )
                return "Points";
        }
    }

    return QVariant();
}

/**
 * @brief RankingsModel::rowCount
 * @param parent
 * @return
 */
int RankingsModel::rowCount( const QModelIndex &parent ) const {
    if ( parent.isValid())
        return 0;

    return Rankings::instance()->list.count();
}

/**
 * @brief RankingsModel::columnCount
 * @param parent
 * @return
 */
int RankingsModel::columnCount( const QModelIndex &parent ) const {
    if ( parent.isValid())
        return 0;

    return 6;
}

/**
 * @brief RankingsModel::data
 * @param index
 * @param role
 * @return
 */
QVariant RankingsModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( role == Qt::DisplayRole ) {
        if ( index.column() == 0 )
            return Rankings::instance()->list.at( index.row()).title;

        if ( index.column() == 1 )
            return Rankings::instance()->list.at( index.row()).completedTasks;

        if ( index.column() == 2 )
            return Rankings::instance()->list.at( index.row()).combos;

        if ( index.column() == 3 )
            return Rankings::instance()->list.at( index.row()).comboTasks;

        if ( index.column() == 4 )
            return Rankings::instance()->list.at( index.row()).penalty;

        if ( index.column() == 5 )
            return Rankings::instance()->list.at( index.row()).points;
    }

    return QVariant();
}
