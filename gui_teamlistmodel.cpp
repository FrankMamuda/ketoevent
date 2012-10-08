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
#include "gui_teamlistmodel.h"
#include <QFont>

/*
================
data
================
*/
QVariant Gui_TeamListModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= m.teamList.count())
        return QVariant();

    // just supply view with team names from global list
    if ( role == Qt::DisplayRole )
        return m.teamList.at( index.row())->name();
#if 0
    // looks ugly
    else if ( role == Qt::FontRole ) {
        QFont font;
        font.setPointSize( 12 );
        return font;
    }
#endif
    else
        return QVariant();
}

/*
================
flags
================
*/
Qt::ItemFlags Gui_TeamListModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}
