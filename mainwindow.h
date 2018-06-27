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
#include <QMainWindow>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class MainWindow;
}

//
// classes
//
class Id;

/**
 * @brief The MainWindow class
 */
class MainWindow final : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY( MainWindow )

public:
    virtual ~MainWindow();
    static MainWindow *instance() { static MainWindow *instance( new MainWindow()); return instance; }
    Id currentEventId() const;
    Id currentTeamId() const;

private slots:
    void on_buttonAdd_clicked();
    void on_buttonRemove_clicked();
    void on_buttonRename_clicked();
    void on_comboEvent_currentIndexChanged( int index );
    void on_comboTeam_currentIndexChanged( int index );
    void on_actionEditor_triggered();
    void on_actionTasks_triggered();
    void on_actionRankings_triggered();
    void on_actionSettings_triggered();
    void on_actionConsole_triggered();
    void updateTasks();
    void on_actionCombos_triggered();

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    explicit MainWindow( QWidget *parent = nullptr );
    Ui::MainWindow *ui;
};
