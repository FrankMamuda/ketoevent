/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

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
#include "gui_console.h"
#include "ui_gui_console.h"


/*
================
construct
================
*/
Gui_Console::Gui_Console( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Console ) {
    ui->setupUi( this );
    this->setWindowFlags( Qt::CustomizeWindowHint );
    this->ui->screen->clear();
}

/*
================
destruct
================
*/
Gui_Console::~Gui_Console() {
    delete ui;
}

/*
================
mousePressEvent
================
*/
void Gui_Console::mousePressEvent( QMouseEvent *eventPtr ){
   this->m_windowPos = eventPtr->pos();
}

/*
================
mouseMoveEvent
================
*/
void Gui_Console::mouseMoveEvent( QMouseEvent *eventPtr ) {
    QPoint out;

    if ( eventPtr->buttons() && Qt::LeftButton )
        this->move( this->pos() + eventPtr->pos() - this->m_windowPos );
}

/*
================
print
================
*/
void Gui_Console::print( const QString &msg ) {
    this->ui->screen->append( msg );
}
