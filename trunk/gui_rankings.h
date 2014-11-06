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

#ifndef GUI_RANKINGS_H
#define GUI_RANKINGS_H

//
// includes
//
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "gui_settingsdialog.h"

//
// namespaces
//
namespace Ui {
class Gui_Rankings;
}

//
// class: RankingsSortModel
//
class RankingsSortModel : public QSortFilterProxyModel {
public:
    RankingsSortModel( QObject *parent ) : QSortFilterProxyModel( parent ) {}
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const {
        QVariant leftData = this->sourceModel()->data( left );
        QVariant rightData = sourceModel()->data( right );
        bool n1, n2;
        int num1, num2;

        num1 = leftData.toInt( &n1 );
        num2 = rightData.toInt( &n2 );

        // these must be integers or strings
        if ( n1 && n2 )
            return num1 < num2;
        else
            return QString::localeAwareCompare( leftData.toString(), rightData.toString()) < 0;
    }
};

//
// class: Gui_Rankings
//
class Gui_Rankings : public Gui_SettingsDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Team rankings dialog" )

public:
    explicit Gui_Rankings( QWidget *parent = 0 );
    ~Gui_Rankings();
    const static unsigned int NumRankingColumns = 9;
    enum Columns {
        Rank = 0,
        TeamName,
        Tasks,
        Combos,
        Total,
        Time,
        Penalty,
        Reviewer,
        Points
    };

private slots:
    void rescaleWindow();
    void calculateStatistics();
    void on_exportButton_clicked();
    void fillData();
    void bindVars();

private:
    Ui::Gui_Rankings *ui;
    QStringList columnHeaders;
    QStandardItemModel *modelPtr;
    RankingsSortModel *proxyModel;
};

#endif // GUI_RANKINGS_H
