/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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

#ifndef GUI_RANKINGMODEL_H
#define GUI_RANKINGMODEL_H

//
// includes
//
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "main.h"

//
// namespaces
//
namespace Rankings {
    const static unsigned int NumRankingColumns = 8;
}

//
// class: Gui_RankingModel
//
class Gui_RankingModel : public QAbstractTableModel {
    Q_OBJECT
    Q_ENUMS( Columns )

public:
    enum Columns {
        TeamName = 0,
        Tasks,
        Challenges,
        Combos,
        Grade,
        Time,
        Penalty,
        Points
    };
    Gui_RankingModel() { }

    // reimplemented functions
    int rowCount( const QModelIndex & ) const { return m.teamList.count(); }
    int columnCount( const QModelIndex & ) const { return Rankings::NumRankingColumns; }
    QVariant data( const QModelIndex &, int  ) const;
    QVariant headerData( int, Qt::Orientation, int  ) const;
    Qt::ItemFlags flags( const QModelIndex & ) const;

    // this resets whole model
    void resetModelData() { this->beginResetModel(); this->endResetModel(); }
};

#endif // GUI_RANKINGMODEL_H
