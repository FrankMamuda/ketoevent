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
#include "combomodel.h"
#include "log.h"
#include "mainwindow.h"
#include "task.h"
#include "event.h"

// initialize colors
QList<QRgb> ComboModel::colourList = ( QList<QRgb>() << 1073774592 << 1082195712 << 1082163200 << 1090453632 << 1082130687 << 1073774720 << 1073807232 );

/**
 * @brief ComboModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ComboModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid())
        return QVariant();

    if ( role == Qt::BackgroundRole )
        return QColor::fromRgba( this->colours[this->data( index, Qt::DisplayRole ).toString()] );

    return QStringListModel::data( index, role );
}

/**
 * @brief ComboModel::reset
 */
#include <QDebug>
void ComboModel::reset( const Id &id ) {
    QStringList list;

    // clear junk
    this->beginResetModel();
    this->map.clear();
    this->colours.clear();

    // build comboId/taskName map and taskName/colour map
    qDebug() << "TEST" << Log::instance()->count();
    for ( int y = 0; y < Log::instance()->count(); y++ ) {
        if ( Log::instance()->teamId( y ) == id ) {
            const Id comboId = Log::instance()->comboId( y );
            const QString taskName( Task::instance()->name( Task::instance()->row( Log::instance()->taskId( y ))));

            if ( comboId > Id::Invalid ) {
                this->map.insertMulti( comboId, taskName );
                this->colours[taskName] = ComboModel::colourForId( static_cast<int>( comboId ));
            }
        }
    }

    // build display list
    this->combos = this->map.uniqueKeys().count();
    this->points = 0;
    foreach ( const Id &comboId, this->map.uniqueKeys()) {
        const QStringList taskNames( this->map.values( comboId ));
        const int count = taskNames.count();

        if ( count == 2 )
            this->points += EventTable::DefaultComboOfTwo;

        if ( count == 3 )
            this->points += EventTable::DefaultComboOfThree;

        if ( count >= 4 )
            this->points += EventTable::DefaultComboOfFourAndMore;

        foreach ( const QString &taskName, taskNames )
            list << taskName;
    }

    // set string list
    this->setStringList( list );
    this->endResetModel();
}
