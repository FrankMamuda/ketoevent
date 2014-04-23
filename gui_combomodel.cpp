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
#include "gui_combomodel.h"
#include "gui_combos.h"

/*
================
data
================
*/
QVariant Gui_ComboModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    Gui_Combos *cPtr = qobject_cast<Gui_Combos *>( this->listParent );
    if ( cPtr == NULL )
        return 0;

    // must have taskListfiltered or smth
    // preferrably unsorted from recently logged
    // +search
    if ( index.row() >= cPtr->logListSorted.count())
        return QVariant();

    if ( role == Qt::DisplayRole )
        return m.taskForId( cPtr->logListSorted.at( index.row())->taskId())->name();
    else if ( role == Qt::UserRole )
        return cPtr->logListSorted.at( index.row())->id();
    else if ( role == Qt::BackgroundColorRole )
        return QVariant();//QColor::red();
    else
        return QVariant();
}

/*
================
flags
================
*/
Qt::ItemFlags Gui_ComboModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}


int Gui_ComboModel::rowCount( const QModelIndex & ) const {
    Gui_Combos *cPtr = qobject_cast<Gui_Combos *>( this->listParent );
    if ( cPtr == NULL )
        return 0;

    return cPtr->logListSorted.count();
}

