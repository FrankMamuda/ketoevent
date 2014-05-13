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
#include "gui_tasklistmodel.h"
#include <QFont>

/*
================
data
================
*/
QVariant Gui_TaskListModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( index.row() >= m.currentEvent()->taskList.count())
        return QVariant();

    // just supply view with team names from global list
    if ( role == Qt::DisplayRole ) {
        TaskEntry *taskPtr = m.currentEvent()->taskList.at( index.row());

        if ( taskPtr->type() == TaskEntry::Multi )
            return QString( "%1 (%2x%3)" ).arg( taskPtr->name()).arg( taskPtr->points()).arg( taskPtr->multi());
        else
            return QString( "%1 (%2)" ).arg( taskPtr->name()).arg( taskPtr->points());
    } else if ( role == Qt::FontRole ) {
        TaskEntry *taskPtr = m.currentEvent()->taskList.at( index.row());
        QFont font;

        if ( taskPtr->style() == TaskEntry::Bold )
            font.setBold( true );
        else if ( taskPtr->style() == TaskEntry::Italic )
            font.setItalic( true );

        return font;
    } else if ( role == Qt::UserRole ) {
        return m.currentEvent()->taskList.at( index.row())->id();
    }

    return QVariant();
}

/*
================
flags
================
*/
Qt::ItemFlags Gui_TaskListModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}

