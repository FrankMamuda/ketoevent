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
/* includes
*/
#include <QMessageBox>
#include "about.h"
#include "ui_about.h"

/**
 * @brief About::About
 * @param parent
 */
About::About( QWidget *parent ) : QDialog( parent ), ui( new Ui::About ) {
    this->ui->setupUi( this );
    QPushButton::connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );
    QPushButton::connect( this->ui->qtButton, &QPushButton::clicked, [ this ]() { QMessageBox::aboutQt( this ); } );
}

/**
 * @brief About::~About
 */
About::~About() {
    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    this->disconnect( this->ui->qtButton, SIGNAL( clicked()));
    delete this->ui;
}
