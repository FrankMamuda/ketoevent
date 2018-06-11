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
#include <QFutureWatcher>
#include <QMainWindow>
#include <QProgressDialog>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class Rankings;
}

/**
 * @brief The Rankings class
 */
class Rankings final : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY( Rankings )

public:
    static Rankings *instance() { static Rankings *instance( new Rankings()); return instance; }
    virtual ~Rankings();

private slots:
    void on_actionUpdate_triggered();

private:
    explicit Rankings( QWidget *parent = nullptr );
    QProgressDialog progress;
    QFutureWatcher<void> futureWatcher;
    Ui::Rankings *ui;
};
