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
#include "combomodel.h"
#include "combodialog.h"
#include "taskwidget.h"
#include <QColor>
#include "main.h"

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

/**
 * @brief colourForId
 * @param id
 * @return
 */
static comboColour_t colourForId( int id ) {
    if ( id < 0 )
        return colours[0];

    if ( id >= numColors )
        return colourForId( id - numColors );

    return colours[id];
}

/**
 * @brief Gui_ComboModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ComboModel::data( const QModelIndex &index, int role ) const {
    ComboDialog *comboDialog;

    if ( !index.isValid())
        return QVariant();

    // get dialog parent
    comboDialog = qobject_cast<ComboDialog *>( this->listParent );
    if ( comboDialog == nullptr )
        return 0;

    // get row count from parent
    if ( index.row() >= comboDialog->logListSorted.count())
        return QVariant();

    if ( role == Qt::DisplayRole )
        return Task::forId( comboDialog->logListSorted.at( index.row())->taskId())->name();
    else if ( role == Qt::UserRole )
        return comboDialog->logListSorted.at( index.row())->id();
    else if ( role == Qt::BackgroundColorRole ) {
        // make sets of combos **colourful**
        // EGH WHAT? FIXME?
        comboColour_t colour = { 1, 1, 1 };

        if ( !comboDialog->logListSorted.isEmpty())
            colour = colourForId( TaskWidget::getRelativeComboId( comboDialog->logListSorted.at( index.row())->comboId(), comboDialog->logListSorted.first()->teamId() ));

        //comboColour_t colour = colourForId( TaskWidget::getRelativeComboId( cPtr->logListSorted.at( index.row())->comboId(), cPtr->logListSorted.first()->teamId()));
        return QColor( colour.r, colour.g, colour.b, 64 );
    } else
        return QVariant();
}

/**
 * @brief Gui_ComboModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags ComboModel::flags( const QModelIndex &index ) const {
    if ( !index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags( index );
}

/**
 * @brief Gui_ComboModel::rowCount
 * @return
 */
int ComboModel::rowCount( const QModelIndex & ) const {
    ComboDialog *comboDialog;

    // get dialog parent            
    comboDialog  = qobject_cast<ComboDialog *>( this->listParent );
    if ( comboDialog == nullptr )
        return 0;

    // get row count from parent
    return comboDialog->logListSorted.count();
}
