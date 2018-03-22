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

#pragma once

//
// includes
//
#include <QStringListModel>
#include "main.h"

/**
 * @brief The TeamListModel class
 */
class TeamListModel : public QStringListModel {
    Q_OBJECT
    Q_CLASSINFO( "description", "Team view engine" )

public:
    TeamListModel( QObject *parentPtr = 0 ) : QStringListModel( parentPtr ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const { return Event::active()->teamList.count(); }
    QVariant data( const QModelIndex &, int ) const;
    Qt::ItemFlags flags( const QModelIndex & ) const;

    // this resets whole model based on data in Main::instance()->teamList
    void beginReset() { this->beginResetModel(); }
    void endReset() { Main::instance()->sort( Main::Teams ); this->endResetModel(); }
};
