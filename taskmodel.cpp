/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "tasklistmodel.h"
#include <QFont>

/**
 * @brief TaskListModel::data
 * @param index
 * @param role
 * @return
 */
QVariant TaskListModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= Event::active()->taskList.count())
        return QVariant();

    // just supply view with team names from global list
    if ( role == Qt::DisplayRole ) {
        Task *taskPtr = Event::active()->taskList.at( index.row());

        if ( taskPtr->type() == Task::Multi )
            return QString( "%1 (%2x%3)" ).arg( taskPtr->name()).arg( taskPtr->points()).arg( taskPtr->multi());
        else
            return QString( "%1 (%2)" ).arg( taskPtr->name()).arg( taskPtr->points());
    } else if ( role == Qt::FontRole ) {
        Task *taskPtr = Event::active()->taskList.at( index.row());
        QFont font;

        if ( taskPtr->style() == Task::Bold )
            font.setBold( true );
        else if ( taskPtr->style() == Task::Italic )
            font.setItalic( true );

        return font;
    } else if ( role == Qt::UserRole ) {
        return Event::active()->taskList.at( index.row())->id();
    }

    return QVariant();
}

/**
 * @brief TaskListModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags TaskListModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}

