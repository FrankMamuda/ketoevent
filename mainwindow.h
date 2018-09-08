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
#include <QCompleter>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTimeEdit>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class MainWindow;
}

//
// classes
//
enum class Id;
Q_DECLARE_METATYPE( Id )

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
    Id currentComboId() const { return this->m_currentCombo; }
    bool isComboModeActive() const { return this->m_comboMode; }
    QModelIndex proxyIndex( const QModelIndex &index ) const;

public slots:
    void setCurrentTeam( const Id &id );
    void setTaskFilter( bool filterByCombo = false, Id comboId = static_cast<Id>( -1 ));

private slots:
    void on_actionTasks_triggered();
    void on_actionRankings_triggered();
    void on_actionSettings_triggered();
    void on_actionConsole_triggered();
    void on_actionCombos_triggered();
    void on_eventCombo_currentIndexChanged( int index );
    void on_teamCombo_currentIndexChanged( int index );
    void on_actionTeams_triggered();
    void on_actionAddQuick_triggered();

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    explicit MainWindow( QWidget *parent = nullptr );
    Ui::MainWindow *ui;
    QCompleter completer;
    QSortFilterProxyModel *filter;
    QWidget *spacer;
    QWidget *quickSpacerLeft;
    QWidget *quickSpacerRight;
    QTimeEdit *timeEdit;
    Id m_currentCombo = static_cast<Id>( -1 );
    bool m_comboMode = false;
};
