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
#include "team.h"
#include "ui_rankings.h"

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
            if ( section == Rank )
                return QT_TR_NOOP_UTF8( "Rank" );

            if ( section == TeamTitle )
                return QT_TR_NOOP_UTF8( "Team\ntitle" );

            if ( section == Completed )
                return QT_TR_NOOP_UTF8( "Completed" );

            if ( section == Combos )
                return QT_TR_NOOP_UTF8( "Combos" );

            if ( section == Combined )
                return QT_TR_NOOP_UTF8( "Combined" );

            if ( section == Penalty )
                return QT_TR_NOOP_UTF8( "Penalty" );

            if ( section == Points )
                return QT_TR_NOOP_UTF8( "Points" );
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

    return ColumnCount;
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
        if ( index.column() == Rank )
            return Rankings::instance()->list.at( index.row()).rank;

        if ( index.column() == TeamTitle ) {
            if ( !Rankings::instance()->isDisplayingCurrentTeam()) {
                return Rankings::instance()->list.at( index.row()).title;
            } else {
                const int team = Rankings::instance()->ui->teamCombo->currentIndex();
                const QString title( Team::instance()->title( team ));

                if ( !QString::compare( title, Rankings::instance()->list.at( index.row()).title ))
                    return Rankings::instance()->list.at( index.row()).title;
            }
        }

        if ( index.column() == Completed )
            return Rankings::instance()->list.at( index.row()).completedTasks;

        if ( index.column() == Combos )
            return Rankings::instance()->list.at( index.row()).combos;

        if ( index.column() == Combined )
            return Rankings::instance()->list.at( index.row()).comboTasks;

        if ( index.column() == Penalty )
            return Rankings::instance()->list.at( index.row()).penalty;

        if ( index.column() == Points )
            return Rankings::instance()->list.at( index.row()).points;
    }

    if ( role == Qt::TextAlignmentRole )
        return Qt::AlignCenter;

    if ( role == Qt::TextColorRole && index.column() == Penalty && Rankings::instance()->list.at( index.row()).penalty > 0 )
        return QColor( Qt::red );

    return QVariant();
}