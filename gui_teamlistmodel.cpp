/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "gui_teamlistmodel.h"

/**
 * @brief Gui_TeamListModel::data
 * @param index
 * @param role
 * @return
 */
QVariant Gui_TeamListModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= m.currentEvent()->teamList.count())
        return QVariant();

    if ( role == Qt::DisplayRole )
        return m.currentEvent()->teamList.at( index.row())->name();
    else if ( role == Qt::UserRole )
        return m.currentEvent()->teamList.at( index.row())->id();
    else
        return QVariant();
}

/**
 * @brief Gui_TeamListModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags Gui_TeamListModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}


