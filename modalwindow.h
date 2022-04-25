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
#include "mainwindow.h"
#include <QKeyEvent>

/**
 * @brief The ModalWindow class
 */
class ModalWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ModalWindow( QWidget *parent = nullptr ) : QMainWindow( parent ) { this->setWindowModality( Qt::ApplicationModal ); }
    ~ModalWindow() override = default;

protected:
    /**
     * @brief showEvent
     * @param event
     */
    void showEvent( QShowEvent *event ) override { MainWindow::instance().setDisabled( true ); QMainWindow::showEvent( event ); }

    /**
     * @brief hideEvent
     * @param event
     */
    void hideEvent( QHideEvent *event ) override {
        MainWindow::instance().setEnabled( true );
        // relock ui elements if required
        MainWindow::instance().setLock();
        QMainWindow::hideEvent( event );
    }

    /**
     * @brief MainWindow::keyPressEvent
     * @param event
     */
    void keyPressEvent( QKeyEvent *event ) override {
        if ( event->key() == Qt::Key_Escape )
            this->hide();

        QMainWindow::keyPressEvent( event );
    }
};
