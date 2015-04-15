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
#include "cmd.h"

#ifdef APPLET_DEBUG

/*
================
construct
================
*/
Gui_Console::Gui_Console( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Console ) {
    ui->setupUi( this );
    this->setWindowFlags( Qt::CustomizeWindowHint );
    this->ui->screen->clear();

    // install event filter
    this->eventFilter = new ConsoleEventFilter();
    this->ui->input->installEventFilter( this->eventFilter );
}

/*
================
destruct
================
*/
Gui_Console::~Gui_Console() {
    this->ui->input->removeEventFilter( this->eventFilter );
    delete this->eventFilter;
    delete ui;
}

/*
================
completeCommand
================
*/
bool Gui_Console::completeCommand() {
    int match = 0;
    QStringList matchedStrings;
    int y;

    // find matching commands
    foreach( Command *cmdPtr, cmd.cmdList ) {
        if ( cmdPtr->name().startsWith( this->ui->input->text(), Qt::CaseInsensitive ))
            matchedStrings << cmdPtr->name();
    }

    // find matching cvars
    foreach( ConsoleVariable *cvarPtr, m.cvarList ) {
        if ( !QString::compare( cvarPtr->key(), "system/consoleHistory" ))
             continue;

        if ( cvarPtr->key().startsWith( this->ui->input->text(), Qt::CaseInsensitive ))
            matchedStrings << cvarPtr->key();
    }

    // complete to shortest string
    if ( matchedStrings.count() == 1 ) {
        // append extra space (since it's the only match that will likely be follwed by an argument)
        this->ui->input->setText( matchedStrings.first() + " " );
    } else if ( matchedStrings.count() > 1 ) {
        match = 1;
        for ( y = 0; y < matchedStrings.count(); y++ ) {
            // make sure we check string length
            if ( matchedStrings.first().length() == match || matchedStrings.at( y ).length() == match )
                break;

            if ( matchedStrings.first().at( match ) == matchedStrings.at( y ).at( match )) {
                if ( y == matchedStrings.count()-1 ) {
                    match++;
                    y = 0;
                }
            }
        }
        this->ui->input->setText( matchedStrings.first().left( match ));
    } else if ( !matchedStrings.count()) {
        return true;
    }

    // print out suggestions
    m.print( /*Sys::cCyan +*/ this->tr( "Available commands and cvars:\n" ), Main::System );
    foreach ( QString str, matchedStrings ) {
        // check commands
        Command *cmdPtr;
        cmdPtr = cmd.find( str );
        if ( cmdPtr != NULL ) {
            if ( !cmdPtr->description().isEmpty()) {
                m.print( QString( "  \"%1\" - %2\n" ).arg( str, cmdPtr->description()), Main::System);
            } else {
                m.print( QString( "  \"%1\n" ).arg( str ), Main::System);
            }
        }

        // check variables
        ConsoleVariable *cvarPtr = m.cvar( str );

        // perform a variable print or set
        if ( cvarPtr != m.defaultCvar )
            m.print( this->tr( "  \"%1\" is \"%2\"\n" ).arg( cvarPtr->key(), cvarPtr->string()), Main::System );
    }

    // add extra newline
    m.print( "\n", Main::System );

    // done
    return true;
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
eventFilter
================
*/
bool ConsoleEventFilter::eventFilter( QObject *objectPtr, QEvent *eventPtr ) {
    QConsoleEdit *lePtr = qobject_cast<QConsoleEdit *>( objectPtr );
    Gui_Console *cPtr = qobject_cast<Gui_Console *>( objectPtr->parent());
    if ( cPtr == NULL || lePtr == NULL )
        return false;

    if ( lePtr->hasFocus()) {
        if ( eventPtr->type() == QEvent::KeyPress ) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>( eventPtr );

            // history list -> up
            if ( keyEvent->key() == Qt::Key_Up ) {
                if ( !lePtr->history.isEmpty()) {
                    if ( lePtr->historyOffset() < lePtr->history.count() )
                        lePtr->pushHistoryOffset();

                    int offset = lePtr->history.count() - lePtr->historyOffset();

                    if ( offset > 0 )
                        lePtr->setText( lePtr->history.at( offset ));
                    else
                        lePtr->setText( lePtr->history.first());
                }
                return true;

                // history list -> down
            } else if ( keyEvent->key() == Qt::Key_Down ) {
                if ( !lePtr->history.isEmpty()) {
                    int offset;

                    if ( lePtr->historyOffset() > 0 )
                        lePtr->popHistoryOffset();

                    if ( lePtr->historyOffset() == 0 ) {
                        lePtr->clear();
                        return true;
                    }

                    offset = lePtr->history.count() - lePtr->historyOffset();

                    if ( offset < lePtr->history.count())
                        lePtr->setText( lePtr->history.at( offset ));
                    else
                        lePtr->setText( lePtr->history.last());
                }
                return true;

                // complete command
            } else if ( keyEvent->key() == Qt::Key_Tab ) {
                // abort if no text at all
                if ( lePtr->text().isEmpty())
                    return true;

                return cPtr->completeCommand();
            }
        }
    }
    return false;
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
    if ( cmd.execute( this->ui->input->text()))
        this->ui->input->addToHistory( this->ui->input->text());

    // set min offset
    this->ui->input->setHistoryOffset( 0 );
    this->ui->input->clear();
}

/*
================
loadHistory
================
*/
void Gui_Console::loadHistory() {
    this->ui->input->history = m.cvar( "system/consoleHistory" )->string().split( ";" );
}

/*
================
saveHisotry
================
*/
void Gui_Console::saveHisotry() {
    m.cvar( "system/consoleHistory" )->setValue( this->ui->input->history.join( ";" ));
}

#endif
