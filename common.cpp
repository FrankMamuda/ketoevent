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

//
// includes
//
#include "common.h"
#include "main.h"
#include <QMessageBox>
#include "mainwindow.h"

/**
 * @brief Common::print
 * @param msg
 * @param debug
 */
void Common::print( const QString &msg, DebugLevel debug ) {
    QString out = msg;

    if ( msg.endsWith( "\n" ))
        out = msg.left( msg.length()-1 );

#ifdef APPLET_DEBUG
    // EVERYTHING is printed to the console
    if ( Main::instance()->console != nullptr )
        Main::instance()->console->print( out );

    // subsystem messages may be skipped
    if ( debug == NoDebug )
        return;
    else {
        if ( !Main::instance()->debugLevel().testFlag( debug ))
            return;
    }

    // output to QDebug
    qDebug() << out;
#else
    Q_UNUSED( msg );
#endif
}

/**
 * @brief Common::error
 * @param type
 * @param func
 * @param msg
 */
void Common::error( ErrorTypes type, const QString &func, const QString &msg ) {
    MainWindow *guiPtr;
    guiPtr = qobject_cast<MainWindow*>( Main::instance()->parent());
    QString dialogMsg;

    // failsafe
    if ( !msg.length())
        return;

    // capitalize for message boxes
    dialogMsg = msg;
    dialogMsg.replace( 0, 1, dialogMsg.at( 0 ).toUpper());

    if ( type == FatalError ) {
        Common::print( QObject::tr( "FATAL ERROR: %1" ).arg( func + msg ), System );

        if ( guiPtr != nullptr ) {
            guiPtr->lock();
            QMessageBox msgBox;
            msgBox.setWindowTitle( QObject::tr( "Fatal error" ));
            msgBox.setText( dialogMsg + "\n" + QObject::tr( "Do you want to reset the database (requires restart)?" ));
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msgBox.setIcon( QMessageBox::Critical );
            int state = msgBox.exec();

            // check options
            switch ( state ) {
            case QMessageBox::Yes:
                Database::unload();
                QFile::rename( Variable::instance()->string( "databasePath" ), QString( "%1_badDB_%2.db" ).arg( Variable::instance()->string( "databasePath" ).remove( ".db" )).arg( QDateTime::currentDateTime().toString( "hhmmss_ddMM" )));
                guiPtr->close();
                break;

            case QMessageBox::No:
            default:
                ;
            }
        } else {
            exit( 0 );
        }
    } else {
        if ( guiPtr != nullptr )
            QMessageBox::warning( guiPtr, "Error", dialogMsg, QMessageBox::Close );

        Common::print( QObject::tr( "ERROR: %1" ).arg( func + msg ), System );
    }
}
