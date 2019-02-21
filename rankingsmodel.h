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

#pragma once

//
// includes
//
#include <QAbstractTableModel>

/**
 * @brief The RankingsModel class
 */
class RankingsModel : public QAbstractTableModel {
    Q_OBJECT
    Q_ENUMS( Columns )

public:
    enum Columns {
        NoColumn = -1,
        Rank,
        TeamTitle,
        Completed,
        Combos,
        Combined,
        Penalty,

#ifdef KK6_SPECIAL
        Regular,
        Special0,
        Special1,
#endif

        Points,

        // do not remove
        ColumnCount
    };

    explicit RankingsModel( QObject *parent = nullptr ) : QAbstractTableModel( parent ) { }
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex()) const override;
    int columnCount( const QModelIndex &parent = QModelIndex()) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    void reset() { this->beginResetModel(); this->endResetModel(); }
};

// declare enums
Q_DECLARE_METATYPE( RankingsModel::Columns )
