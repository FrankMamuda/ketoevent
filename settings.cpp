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

//
// includes
//
#include "main.h"
#include "settings.h"
#include "ui_settings.h"
#include "variable.h"
#include <QFileDialog>
#include <QMessageBox>

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings() : ui( new Ui::Settings ) {
    this->setWindowModality( Qt::ApplicationModal );
    this->ui->setupUi( this );
    this->ui->closeButton->setIcon( QIcon( ":/icons/close" ));
    this->variables << Variable::instance()->bind( "reviewerName", this->ui->reviewerEdit );
    this->variables << Variable::instance()->bind( "sortByType", this->ui->sortByTypeCheck );

    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );

    // handle database path
    this->connect( this->ui->pathButton, &QPushButton::clicked, [ this ]() {
        const QString fileName( QFileDialog::getSaveFileName
                                ( this, this->tr( "Open database" ),
                                  QFileInfo( Variable::instance()->string( "databasePath" )).absolutePath(),
                                  this->tr( "Database (*.db *.sqlite)" ), nullptr, QFileDialog::DontConfirmOverwrite ));

        if ( fileName.isEmpty()) {
            QMessageBox::warning( this,
                                  this->tr( "Settings" ),
                                  this->tr( "Invalid database selection" ),
                                  QMessageBox::Close );
            return;
        }

        QMessageBox::warning( this,
                              this->tr( "Settings" ),
                              this->tr( "Application will be restarted" ),
                              QMessageBox::Ok );
        Variable::instance()->setString( "databasePath", fileName );
        QApplication::quit();
    } );

    // bind database path to edit
    Variable::instance()->bind( "databasePath", this->ui->pathEdit );
    Variable::instance()->bind( "backup/enabled", this->ui->backupCheck );
    Variable::instance()->bind( "backup/changes", this->ui->backupValue );

    // set window icon
    this->setWindowIcon( QIcon( ":/icons/overflow" ));

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    // unbind vars
    foreach ( const QString &key, this->variables )
        Variable::instance()->unbind( key );
    this->variables.clear();

    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    delete this->ui;
}
