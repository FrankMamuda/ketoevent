/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

#ifndef GUI_DIALOG_H
#define GUI_DIALOG_H

//
// includes
//
#include <QDialog>
#include "settingsvariable.h"
#include "main.h"
#include <QMainWindow>

//
// inherited by Gui_Event and Gui_Settings
//

/**
 * @brief The Gui_Dialog class
 */
class Gui_Dialog : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY( bool variablesLocked READ variablesLocked WRITE lockVariables )
    Q_CLASSINFO( "description", "Settings base class" )

public:
    Gui_Dialog( QWidget *parent ) : QMainWindow( parent ) { this->setWindowModality( Qt::ApplicationModal ); }
    bool variablesLocked() const { return this->m_variablesLocked; }
    enum CloseSignals {
        Accepted = 0,
        Rejected = 1,
        Closed = 2
    };

protected:
    void closeEvent( QCloseEvent *ePtr ) { QMainWindow::closeEvent( ePtr ); emit this->closeSignal( Closed ); }

public slots:
    void lockVariables( bool lock = true ) { this->m_variablesLocked = lock; }
    void unbindVars() { if ( !m.isInitialised()) return; foreach ( SettingsVariable* svar, this->svars ) svar->unbind(); svars.clear(); }
    void bindVariable( const QString &key, QObject *object ) { this->svars << m.svar( key ); m.svar( key )->bind( object, qobject_cast<QObject*>( this )); }
    void updateVars() { foreach ( SettingsVariable *svar, this->svars ) svar->setState(); }
    void onAccepted() { this->hide(); emit this->closeSignal( Accepted ); }
    void onRejected() { this->hide(); emit this->closeSignal( Rejected ); }

signals:
    void closeSignal( int );

private:
    bool m_variablesLocked;
    QList <SettingsVariable*> svars;
};

#endif // GUI_DIALOG_H
