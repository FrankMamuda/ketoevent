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
#include "main.h"

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

/*
================
input->returnPressed
================
*/
void Gui_Console::on_input_returnPressed() {
    QString cmdString = this->ui->input->text();
    QStringList tokenList = cmdString.split( " " );
    QString cmd = tokenList.at( 0 );

    // print out
    this->print( "> " + cmdString );

    // for now, just use a highly simplified algorithm
    if ( !QString::compare( cmd, "cv_list", Qt::CaseInsensitive )) {
        if ( !m.cvarList.isEmpty())
            this->print( "console variables:" );

        foreach ( ConsoleVariable *cvarPtr, m.cvarList ) {
            if ( QString::compare( cvarPtr->defaultValue().toString(), cvarPtr->value().toString()), Qt::CaseInsensitive )
                this->print( QString( "  \"%1\" is \"%2\", default - \"%3\"" ).arg( cvarPtr->key()).arg( cvarPtr->value().toString()).arg( cvarPtr->defaultValue().toString()));
            else
                this->print( QString( "  \"%1\" is \"%2\"" ).arg( cvarPtr->key()).arg( cvarPtr->value().toString()));
        }
    } else if ( !QString::compare( cmd, "cv_set", Qt::CaseInsensitive )) {
        // TODO: must properly tokenize string
        if ( tokenList.count() != 3 ) {
            this->print( "invalid syntax - cv_set [key] [value]" );
        } else {
            ConsoleVariable *cvarPtr = m.cvar( tokenList.at( 1 ));
            if ( QString::compare( cvarPtr->key(), tokenList.at( 1 ), Qt::CaseInsensitive ))
                this->print( QString( "no such cvar - \"%1\"" ).arg( tokenList.at( 1 )));
            else {
                this->print( QString( "setting \"%1\" to \"%2\"" ).arg( cvarPtr->key()).arg( tokenList.at( 2 )));
                cvarPtr->setValue( tokenList.at( 2 ));
            }
        }
    } else if ( !QString::compare( cmd, "db_info", Qt::CaseInsensitive )) {
        this->print( QString( "events - %1, teams - %2 (%3), tasks - %4 (%5), logs - %6" )
                     .arg( m.base.eventList.count())
                     .arg( m.currentEvent()->teamList.count())
                     .arg( m.base.teamList.count())
                     .arg( m.currentEvent()->taskList.count())
                     .arg( m.base.taskList.count())
                     .arg( m.base.logList.count()));
    } else
        this->print( QString( "unknown command - \"%1\"" ).arg( cmd ));

    this->ui->input->clear();
}
