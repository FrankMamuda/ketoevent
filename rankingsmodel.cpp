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
                return QObject::tr( "Rank" );

            if ( section == TeamTitle )
                return QObject::tr( "Team\ntitle" );

            if ( section == Completed )
                return QObject::tr( "Completed" );

            if ( section == Combos )
                return QObject::tr( "Combos" );

            if ( section == Combined )
                return QObject::tr( "Combined" );

            if ( section == Penalty )
                return QObject::tr( "Penalty" );

#ifdef KK6_SPECIAL
            if ( section == Regular )
                return QObject::tr( "Regular" );

            if ( section == Special0 )
                return QObject::tr( "FTF" );

            if ( section == Special1 )
                return QObject::tr( "Special" );

#endif
            if ( section == Points )
                return QObject::tr( "Points" );
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
                const Row team = Team::instance()->row( Rankings::instance()->ui->teamCombo->currentIndex());
                if ( team == Row::Invalid )
                    return QVariant();

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

#ifdef KK6_SPECIAL
        if ( index.column() == Regular )
            return Rankings::instance()->list.at( index.row()).points -
                   Rankings::instance()->list.at( index.row()).specialPoints0 -
                   Rankings::instance()->list.at( index.row()).specialPoints1;

        if ( index.column() == Special0 )
            return Rankings::instance()->list.at( index.row()).specialPoints0;

        if ( index.column() == Special1 )
            return Rankings::instance()->list.at( index.row()).specialPoints1;
#endif

        if ( index.column() == Points )
            return Rankings::instance()->list.at( index.row()).points;
    }

    if ( role == Qt::TextAlignmentRole )
        return Qt::AlignCenter;

    if ( role == Qt::TextColorRole && index.column() == Penalty && Rankings::instance()->list.at( index.row()).penalty > 0 )
        return QColor( Qt::red );

    return QVariant();
}
