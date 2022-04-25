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

/*
 * includes
 */
#include "main.h"
#include "settings.h"
#include "theme.h"
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

    // setup pixmaps
    this->ui->reviewerPixmap->setPixmap( QIcon::fromTheme( "ketone" ).pixmap( 16, 16 ));



    const QStringList themes( Theme::availableThemes().keys());
    this->ui->themeCombo->clear();
    for ( const QString &themeName : themes )
        this->ui->themeCombo->addItem( themeName, themeName );

    Settings::connect( this->ui->overrideCheck, &QCheckBox::toggled,
                   [ this ]( bool checked ) { this->ui->themeCombo->setEnabled( checked ); } );

    // bind variables
    this->variables << Variable::instance().bind( "overrideTheme", this->ui->overrideCheck );
    this->variables << Variable::instance().bind( "theme", this->ui->themeCombo );
    this->variables << Variable::instance().bind( "reviewerName", this->ui->reviewerEdit );
    this->variables << Variable::instance().bind( "sortByType", this->ui->sortByTypeCheck );

    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );

    // handle database path
    this->connect( this->ui->pathButton, &QPushButton::clicked, [ this ]() {
        const QString fileName( QFileDialog::getSaveFileName
                                ( this, this->tr( "Open database" ),
                                  QFileInfo( Variable::string( "databasePath" )).absolutePath(),
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
        Variable::setString( "databasePath", fileName );
        QApplication::quit();
    } );

    // bind database path to edit
    Variable::instance().bind( "databasePath", this->ui->pathEdit );
    Variable::instance().bind( "backup/enabled", this->ui->backupCheck );
    Variable::instance().bind( "backup/changes", this->ui->backupValue );
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    // unbind vars
    for ( const QString &key : qAsConst( this->variables ))
        Variable::instance().unbind( key );
    this->variables.clear();

    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    delete this->ui;
}
