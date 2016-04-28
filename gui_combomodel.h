/*
===========================================================================
Copyright (C) 2013-2016 Avotu Briezhaudzetava

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

#ifndef GUI_COMBOMODEL_H
#define GUI_COMBOMODEL_H

//
// includes
//
#include <QStringListModel>
#include <QSortFilterProxyModel>

//
// class: Combo model (gui)
//
class Gui_ComboModel : public QStringListModel {
    Q_OBJECT
    Q_CLASSINFO( "description", "Combination view engine" )

public:
    Gui_ComboModel( QObject *parentPtr = 0 ) : QStringListModel( parentPtr ) { this->listParent = parentPtr; }
    int rowCount( const QModelIndex & = QModelIndex()) const;
    QVariant data( const QModelIndex &, int ) const;
    Qt::ItemFlags flags( const QModelIndex & ) const;

public slots:
    // this resets whole model
    void beginReset() { this->beginResetModel(); }
    void endReset() { this->endResetModel(); }

private:
    QObject *listParent;
};

#endif // GUI_COMBOMODEL_H
