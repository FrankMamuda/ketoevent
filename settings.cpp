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
#include <QCommonStyle>
#include "main.h"
#include "settings.h"
#include "ui_settings.h"
#include "variable.h"

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings() : ui( new Ui::Settings ) {
    QCommonStyle style;

    this->setWindowModality( Qt::ApplicationModal );
    this->ui->setupUi( this );
    this->ui->closeButton->setIcon( style.standardIcon( QStyle::SP_DialogCloseButton ));
    this->variables << Variable::instance()->bind( "reviewerName", this->ui->reviewerEdit );

    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );

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
