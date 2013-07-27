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

#ifndef GUI_RANKINGS_H
#define GUI_RANKINGS_H

//
// includes
//
#include <QDialog>
#include "gui_rankingmodel.h"

//
// namespaces
//
namespace Ui {
    class Gui_Rankings;
}

//
// class: Gui_Rankings
//
class Gui_Rankings : public QDialog {
    Q_OBJECT

public:
    explicit Gui_Rankings( QWidget *parent = 0 );
    ~Gui_Rankings();

public slots:
    void rescaleWindow();

private slots:
    void calculateStatistics();

private:
    Ui::Gui_Rankings *ui;
    Gui_RankingModel *modelPtr;
    QSortFilterProxyModel *proxyModel;
};

#endif // GUI_RANKINGS_H
