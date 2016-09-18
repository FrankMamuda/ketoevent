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

//
// includes
//
#include "gui_settings.h"
#include "ui_gui_settings.h"
#include <QFileDialog>
#include <QTextStream>
#include "main.h"
#include "gui_main.h"

/**
 * @brief Gui_Settings::Gui_Settings
 * @param parent
 */
Gui_Settings::Gui_Settings( QWidget *parent ) : Gui_Dialog( parent ), ui( new Ui::Gui_Settings ) {
    ui->setupUi( this );

    if ( m.isInitialised())
        this->bindVars();
    else
        this->onRejected();
}

/**
 * @brief Gui_Settings::~Gui_Settings
 */
Gui_Settings::~Gui_Settings() {
    this->unbindVars();
    delete ui;
}

/**
 * @brief Gui_Settings::bindVars
 */
void Gui_Settings::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    this->bindVariable( "backup/changes", this->ui->backupChanges );
    this->bindVariable( "backup/perform", this->ui->backupPerform );
    this->bindVariable( "misc/sortTasks", this->ui->sort );
    this->bindVariable( "misc/hilightLogged", this->ui->hilightEntries );
    this->bindVariable( "databasePath", this->ui->dbPath );
    this->bindVariable( "reviewerName", this->ui->rvName );

    // unlock vars
    this->lockVariables( false );
}

/**
 * @brief Gui_Settings::on_backupPerform_stateChanged
 * @param state
 */
void Gui_Settings::on_backupPerform_stateChanged( int state ) {
    if ( state == Qt::Checked )
        this->ui->backupChanges->setEnabled( true );
    else
        this->ui->backupChanges->setDisabled( true );
}

/**
 * @brief Gui_Settings::on_pathButton_clicked
 */
void Gui_Settings::on_pathButton_clicked() {
    Gui_Main *gui;
    QString path, filePath;

    // get filename from dialog
    path = QString( QDir::currentPath() + "/" );
    filePath = QFileDialog::getSaveFileName( this, this->tr( "Select database" ), path, this->tr( "Database (*.db)" ), 0, QFileDialog::DontConfirmOverwrite );

    // check for empty filenames
    if ( filePath.isEmpty())
        return;

    // check if path is valid
    if ( !QFileInfo( filePath ).absoluteDir().isReadable())
        return;

    // append extension
    if ( !filePath.endsWith( ".db" ))
        filePath.append( ".db" );

    // don't reload the same database
    if ( !QString::compare( filePath, m.cvar( "databasePath" )->string()))
        return;

    // store database path
    this->ui->dbPath->setText( filePath );

    // clear data on reload
    m.clearEvent();

    // reload database
    m.reloadDatabase( m.cvar( "databasePath" )->string());

    // reinitialise gui
    gui = qobject_cast<Gui_Main*>( this->parent());
    if ( gui != NULL )
        gui->initialise( true );
}

/**
 * @brief Gui_Settings::on_closeButton_clicked
 */
void Gui_Settings::on_closeButton_clicked() {
    this->onAccepted();
}
