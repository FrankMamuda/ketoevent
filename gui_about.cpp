/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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
#include "gui_about.h"
#include "ui_gui_about.h"

/*
================
construct
================
*/
Gui_About::Gui_About( QWidget *parent ) : QDialog( parent ), m_ui( new Ui::Gui_About ) {
    m_ui->setupUi( this );

    // this is a fixed frame
    this->setSizeGripEnabled( false );

#ifdef Q_OS_MAC
    // fixes ugly mac font
    m_ui->appInfo->setHtml( m_ui->appInfo->toHtml().replace( "font-size:8pt", "font-size:12pt" ));
#endif
}

/*
================
destruct
================
*/
Gui_About::~Gui_About() {
    delete m_ui;
}
