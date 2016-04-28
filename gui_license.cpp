/*
===========================================================================
Copyright (C) 2013-2016 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

//
// includes
//
#include "gui_license.h"
#include "ui_gui_license.h"

/*
================
construct
================
*/
Gui_License::Gui_License( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_License ) {
    ui->setupUi( this );

#ifdef Q_OS_MAC
    // fixes ugly mac font
    this->ui->licenseContainer->setHtml( ui->licenseContainer->toHtml().replace( "font-size:8pt", "font-size:12pt" ));
#endif
}

/*
================
destruct
================
*/
Gui_License::~Gui_License() {
    delete ui;
}
