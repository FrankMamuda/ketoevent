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
#include "settings.h"
#include "ui_settings.h"
#include <QFileDialog>
#include <QTextStream>
#include "main.h"
#include "mainwindow.h"

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings( QWidget *parent ) : Dialog( parent ), ui( new Ui::Settings ) {
    this->ui->setupUi( this );

    if ( Main::instance()->isInitialised()) {
        this->variables << Variable::instance()->bind( "backup/changes", this->ui->backupChanges );
        this->variables << Variable::instance()->bind( "backup/perform", this->ui->backupPerform );
        this->variables << Variable::instance()->bind( "misc/sortTasks", this->ui->sort );
        this->variables << Variable::instance()->bind( "misc/hilightLogged", this->ui->hilightEntries );
        this->variables << Variable::instance()->bind( "databasePath", this->ui->dbPath );
        this->variables << Variable::instance()->bind( "reviewerName", this->ui->rvName );
    } else
        this->onRejected();

    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->onAccepted(); } );
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    foreach ( const QString &key, this->variables )
        Variable::instance()->unbind( key );

    this->variables.clear();
    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    delete this->ui;
}

/**
 * @brief Settings::on_pathButton_clicked
 */
void Settings::on_pathButton_clicked() {
    MainWindow *mainWindow;
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
    if ( !QString::compare( filePath, Variable::instance()->string( "databasePath" )))
        return;

    // store database path
    this->ui->dbPath->setText( filePath );

    // clear data on reload
    Main::instance()->clearEvent();

    // reload database
    Database::reload( Variable::instance()->string( "databasePath" ));

    // reinitialise gui
    mainWindow = qobject_cast<MainWindow*>( this->parent());
    if ( mainWindow != nullptr )
        mainWindow->initialise( true );
}
