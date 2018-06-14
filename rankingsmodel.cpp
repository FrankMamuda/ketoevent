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
 * @brief RankingsModel::RankingsModel
 * @param parent
 */
RankingsModel::RankingsModel( QObject *parent ) : QAbstractTableModel( parent ) {
}

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
                return "Title";

            if ( section == 1 )
                return "Completed";

            if ( section == 2 )
                return "Combos";

            if ( section == 3 )
                return "Combined";

            if ( section == 4 )
                return "Papilduzdevums";

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

    return this->list.count();// Rankings::instance()->map.count();
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
            return this->list.at( index.row()).title; // Rankings::instance()->map.values().at( index.row()).title;

        if ( index.column() == 1 )
            return this->list.at( index.row()).completedTasks; //Rankings::instance()->map.values().at( index.row()).completedTasks;

        if ( index.column() == 2 )
            return this->list.at( index.row()).combos; //Rankings::instance()->map.values().at( index.row()).points;

        if ( index.column() == 3 )
            return this->list.at( index.row()).comboTasks; //Rankings::instance()->map.values().at( index.row()).points;

        if ( index.column() == 4 )
            return this->list.at( index.row()).extra; //Rankings::instance()->map.values().at( index.row()).points;

        if ( index.column() == 5 )
            return this->list.at( index.row()).points; //Rankings::instance()->map.values().at( index.row()).points;
    }

    return QVariant();
}
