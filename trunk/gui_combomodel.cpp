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
#include "taskwidget.h"
#include <QColor>

//
// comboColour_t
//
typedef struct comboColour_s {
    int r;
    int g;
    int b;
} comboColour_t;

// colour table
static comboColour_t colours[] = {
    { 0,  128,    0 },
    { 128, 255,   0 },
    { 128, 128,   0 },
    { 255,   0, 128 },
    { 128,   0, 255 },
    {   0, 128, 128 },
    {   0, 255, 128 },
};
static int numColors = sizeof( colours ) / sizeof( comboColour_t );

/*
================
colourForId
================
*/
static comboColour_t colourForId( int id ) {
    if ( id < 0 )
        return colours[0];

    if ( id >= numColors )
        return colourForId( id - numColors );

    return colours[id];
}

/*
================
data
================
*/
QVariant Gui_ComboModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    // get dialog parent
    GetPtr( Gui_Combos *, cPtr, this->listParent ); TestPtr( cPtr ) return 0;

    // get row count from parent
    if ( index.row() >= cPtr->logListSorted.count())
        return QVariant();

    if ( role == Qt::DisplayRole )
        return m.taskForId( cPtr->logListSorted.at( index.row())->taskId())->name();
    else if ( role == Qt::UserRole )
        return cPtr->logListSorted.at( index.row())->id();
    else if ( role == Qt::BackgroundColorRole ) {
        // make sets of combos **colourful**
        comboColour_t colour = colourForId( TaskWidget::getRelativeComboId( cPtr->logListSorted.at( index.row())->comboId(), cPtr->currentTeamIndex()));
        return QColor( colour.r, colour.g, colour.b, 64 );
    } else
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

/*
================
rowCount
================
*/
int Gui_ComboModel::rowCount( const QModelIndex & ) const {
    // get dialog parent
    GetPtr( Gui_Combos *, cPtr, this->listParent ); TestPtr( cPtr ) return 0;

    // get row count from parent
    return cPtr->logListSorted.count();
}
