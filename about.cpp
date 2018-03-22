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
#include "about.h"
#include "ui_about.h"

/**
 * @brief About::About applet info dialog
 * @param parent parent widget
 */
About::About( QWidget *parent ) : QDialog( parent ), ui( new Ui::About ) {
    ui->setupUi( this );

#ifdef Q_OS_MAC
    // fixes ugly mac font
    this->ui->appInfo->setHtml( this->ui->appInfo->toHtml().replace( "font-size:8pt", "font-size:10pt" ));
#endif
}

/**
 * @brief About::~About destructor
 */
About::~About() {
    delete ui;
}

