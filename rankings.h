/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
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
        title( n ) {}
    QString title;
    int completedTasks = 0;
    int combos = 0;
    int comboTasks = 0;
    int points = 0;
    int penalty = 0;
    int time = 0;
#ifdef KK6_SPECIAL
    // NOTE: hardcoded
    int specialPoints0 = 0;
    int specialPoints1 = 0;
#endif
    int rank = 0;
};

/**
 * @brief The Rankings class
 */
class Rankings final : public ModalWindow {
    Q_OBJECT
    Q_DISABLE_COPY( Rankings )
    friend class RankingsModel;

public:
    // disable move
    Rankings( Rankings&& ) = delete;
    Rankings& operator=( Rankings&& ) = delete;

    static Rankings *instance() { static Rankings *instance( new Rankings()); return instance; }
    ~Rankings() override;
    [[nodiscard]] bool isDisplayingCurrentTeam() const;

private slots:
    void on_actionUpdate_triggered();
    void on_closeButton_clicked() { this->hide(); }
    void on_actionExport_triggered();

protected:
    void showEvent( QShowEvent *event );

private:
    explicit Rankings();
    Ui::Rankings *ui;
    RankingsModel *model;
    QSortFilterProxyModel *proxyModel;
    QList<TeamStatistics> list;
};
