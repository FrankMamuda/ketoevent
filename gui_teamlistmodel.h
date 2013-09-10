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

#ifndef GUI_TEAMLISTMODEL_H
#define GUI_TEAMLISTMODEL_H

//
// includes
//
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include "main.h"

//
// class: Gui_TeamListModel
//
class Gui_TeamListModel : public QStringListModel {
    Q_OBJECT

public:
    Gui_TeamListModel( QObject *parentPtr = 0 ) : QStringListModel( parentPtr ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const { return m.teamList.count(); }
    QVariant data( const QModelIndex &, int ) const;
    Qt::ItemFlags flags( const QModelIndex & ) const;

    // this resets whole model based on data in m.teamList
    void beginReset() { this->beginResetModel(); }
    void endReset() { m.sort( Main::Teams ); this->endResetModel(); }
};

#endif // GUI_TEAMLISTMODEL_H
