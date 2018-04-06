/*
 * Copyright (C) 2013-2018 Factory #12
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
#include <QDialog>
#include "main.h"
#include <QMainWindow>

//
// inherited by EventDialog and Settings
//

/**
 * @brief The Dialog class
 */
class Dialog : public QMainWindow {
    Q_OBJECT
    Q_CLASSINFO( "description", "Settings base class" )

public:
    Dialog( QWidget *parent ) : QMainWindow( parent ) { this->setWindowModality( Qt::ApplicationModal ); }
    enum CloseSignals {
        Accepted = 0,
        Rejected = 1,
        Closed = 2
    };

protected:
    void closeEvent( QCloseEvent *ePtr ) { QMainWindow::closeEvent( ePtr ); emit this->closeSignal( Closed ); }

public slots:
    void onAccepted() { this->hide(); emit this->closeSignal( Accepted ); }
    void onRejected() { this->hide(); emit this->closeSignal( Rejected ); }

signals:
    void closeSignal( int );
};
