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
#include "license.h"
#include "ui_about.h"

//
// namespaces
//
namespace Ui {
class About;
}

/**
 * @brief The About class
 */
class About : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "About dialog" )

public:
    /**
     * @brief About
     * @param parent
     */
    About( QWidget *parent = 0 ) : QDialog( parent ), ui( new Ui::About ) {
        this->ui->setupUi( this );

#ifdef Q_OS_MAC
        // fixes ugly mac font
        this->ui->appInfo->setHtml( this->ui->appInfo->toHtml().replace( "font-size:8pt", "font-size:10pt" ));
#endif
        this->connect( this->ui->licenseButton, &QPushButton::clicked, [ this ]() { License license( this ); license.exec(); } );
    }

    /**
     * @brief About::~About
     */
    ~About() { delete this->ui; }

private:
    Ui::About *ui;
};
