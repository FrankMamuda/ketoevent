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

#pragma once

//
// includes
//
#include "modalwindow.h"
#include <QProgressDialog>
#include <QSortFilterProxyModel>

//
// classes
//
class RankingsModel;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class Rankings;
}

/**
 * @brief The TeamStatistics class
 */
class TeamStatistics final {
public:
    explicit TeamStatistics( const QString &n = QString()) :
        title( n ),
        completedTasks( 0 ),
        combos( 0 ),
        comboTasks( 0 ),
        points( 0 ),
        penalty( 0 ),
        time( 0 ) {}
    QString title;
    int completedTasks;
    int combos;
    int comboTasks;
    int points;
    int penalty;
    int time;
};

/**
 * @brief The Rankings class
 */
class Rankings final : public ModalWindow {
    Q_OBJECT
    Q_DISABLE_COPY( Rankings )
    friend class RankingsModel;

public:
    static Rankings *instance() { static Rankings *instance( new Rankings()); return instance; }
    virtual ~Rankings();

private slots:
    void on_actionUpdate_triggered();
    void on_closeButton_clicked();
    void on_actionExport_triggered();

protected:
    void showEvent( QShowEvent *event );
    void hideEvent( QHideEvent *event );

private:
    explicit Rankings();
    Ui::Rankings *ui;
    QString prevFilter;
    RankingsModel *model;
    QSortFilterProxyModel *proxyModel;
    QList<TeamStatistics> list;
};
