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
#include "console.h"
#include "ui_console.h"
#include "main.h"
#include "cmd.h"
#include <QScrollBar>

#ifdef APPLET_DEBUG

/**
 * @brief Console::Console
 * @param parent
 */
Console::Console( QWidget *parent ) : QDialog( parent ), ui( new Ui::Console ) {
    this->ui->setupUi( this );
    this->setWindowFlags( Qt::CustomizeWindowHint );
    this->ui->screen->clear();

    // install event filter
    this->eventFilter = new ConsoleEventFilter();
    this->ui->input->installEventFilter( this->eventFilter );

    // style the scrollbar
    //this->ui->screen->verticalScrollBar()->setStyleSheet( QString::fromUtf8( "QScrollBar:vertical { border: 0px solid #000000; background:rgb(43, 43, 43); width:10px; margin: 0px 0px 0px 0px; };" ));
}

/**
 * @brief Console::~Console
 */
Console::~Console() {
    this->ui->input->removeEventFilter( this->eventFilter );
    delete this->eventFilter;
    delete ui;
}

/**
 * @brief Console::completeCommand
 * @return
 */
bool Console::completeCommand() {
    int match = 0;
    QStringList matchedStrings;
    int y;

    // find matching commands
    foreach( Command *cmdPtr, Cmd::instance()->cmdList ) {
        if ( cmdPtr->name().startsWith( this->ui->input->text(), Qt::CaseInsensitive ))
            matchedStrings << cmdPtr->name();
    }

    // find matching cvars
    foreach( VariableEntry entry, Variable::instance()->list ) {
        if ( !QString::compare( entry.key(), "system/consoleHistory" ))
             continue;

        if ( entry.key().startsWith( this->ui->input->text(), Qt::CaseInsensitive ))
            matchedStrings << entry.key();
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
    Common::print( this->tr( "Available commands and cvars:\n" ), Common::Console );
    foreach ( QString str, matchedStrings ) {
        // check commands
        Command *cmdPtr;
        cmdPtr = Cmd::instance()->find( str );
        if ( cmdPtr != nullptr ) {
            if ( !cmdPtr->description().isEmpty()) {
                Common::print( QString( "  \"%1\" - %2\n" ).arg( str, cmdPtr->description()), Common::Console);
            } else {
                Common::print( QString( "  \"%1\n" ).arg( str ), Common::Console );
            }
        }

        // check variables
        // NOTE: this is case sensitive
        if ( Variable::instance()->contains( str )) {
            VariableEntry entry;

            entry = Variable::instance()->list[str];
            Common::print( this->tr( "  \"%1\" is \"%2\"\n" ).arg( entry.key(), entry.value().toString()), Common::Console );
        }
    }

    // add extra newline
    Common::print( "\n", Common::Console );

    // done
    return true;
}

/**
 * @brief Console::mousePressEvent
 * @param eventPtr
 */
void Console::mousePressEvent( QMouseEvent *eventPtr ){
    this->m_windowPos = eventPtr->pos();
}

/**
 * @brief Console::mouseMoveEvent
 * @param eventPtr
 */
void Console::mouseMoveEvent( QMouseEvent *eventPtr ) {
    QPoint out;

    if ( eventPtr->buttons() && Qt::LeftButton )
        this->move( this->pos() + eventPtr->pos() - this->m_windowPos );
}

/**
 * @brief ConsoleEventFilter::eventFilter
 * @param objectPtr
 * @param eventPtr
 * @return
 */
bool ConsoleEventFilter::eventFilter( QObject *objectPtr, QEvent *eventPtr ) {
    QConsoleEdit *lePtr = qobject_cast<QConsoleEdit *>( objectPtr );
    Console *cPtr = qobject_cast<Console *>( objectPtr->parent());
    if ( cPtr == nullptr || lePtr == nullptr )
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

/**
 * @brief Console::print
 * @param msg
 */
void Console::print( const QString &msg ) {
    this->ui->screen->append( msg );

    // move cursor
    QTextCursor cursor( this->ui->screen->textCursor());
    cursor.movePosition( QTextCursor::End );
    this->ui->screen->verticalScrollBar()->setValue( this->ui->screen->verticalScrollBar()->maximum());
}

/**
 * @brief Console::on_input_returnPressed
 */
void Console::on_input_returnPressed() {
    if ( Cmd::instance()->execute( this->ui->input->text()))
        this->ui->input->addToHistory( this->ui->input->text());

    // set min offset
    this->ui->input->setHistoryOffset( 0 );
    this->ui->input->clear();
}

/**
 * @brief Console::loadHistory
 */
void Console::loadHistory() {
    this->ui->input->history = Variable::instance()->string( "system/consoleHistory" ).split( ";" );
}

/**
 * @brief Console::saveHisotry
 */
void Console::saveHisotry() {
    Variable::instance()->setValue( "system/consoleHistory", this->ui->input->history.join( ";" ));
}

/**
 * @brief Console::init
 */
void Console::init() {
    // announce
    Common::print( CLMsg + QObject::tr( "initilising console\n" ), Common::Console );

    if ( Main::instance()->console != nullptr )
        return;

    Main::instance()->console = new Console();
    Main::instance()->console->hide();
}

/**
 * @brief show
 */
void Console::show() {
    Main::instance()->console->window()->show();
}

/**
 * @brief show
 */
void Console::hide() {
    Main::instance()->console->window()->hide();
}

#endif
