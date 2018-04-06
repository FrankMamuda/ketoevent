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
    foreach( const QString &name, Cmd::instance()->keys()) {
        if ( name.startsWith( this->ui->input->text()))
            matchedStrings << name;
    }

    // find matching cvars
    foreach( QSharedPointer<Var> entry, Variable::instance()->list ) {
        if ( !QString::compare( entry->key(), "system/consoleHistory" ))
            continue;

        if ( entry->key().startsWith( this->ui->input->text()))
            matchedStrings << entry->key();
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
    qInfo() << this->tr( "Available commands and cvars:" );
    foreach ( const QString &str, matchedStrings ) {
        // check commands
        if ( Cmd::instance()->keys().contains( str )) {
            QString description;

            description = Cmd::instance()->description( str );
            qInfo() << ( !description.isEmpty() ? QString( "  \"%1\" - %2" ).arg( str, description ) : QString( "  \"%1" ).arg( str ));
        }

        // check variables
        // NOTE: this is case sensitive
        if ( Variable::instance()->contains( str )) {
            QSharedPointer<Var> entry;

            entry = Variable::instance()->list[str];
            qInfo() << this->tr( "  \"%1\" is \"%2\"" ).arg( entry->key(), entry->value().toString());
        }
    }

    // add extra newline
    qInfo() << "\n";

    // done
    return true;
}

/**
 * @brief Console::mousePressEvent
 * @param event
 */
void Console::mousePressEvent( QMouseEvent *event ){
    this->m_windowPos = event->pos();
}

/**
 * @brief Console::mouseMoveEvent
 * @param event
 */
void Console::mouseMoveEvent( QMouseEvent *event ) {
    QPoint out;

    if ( event->buttons() && Qt::LeftButton )
        this->move( this->pos() + event->pos() - this->m_windowPos );
}

/**
 * @brief ConsoleEventFilter::eventFilter
 * @param object
 * @param event
 * @return
 */
bool ConsoleEventFilter::eventFilter( QObject *object, QEvent *event ) {
    QConsoleEdit *consoleEdit = qobject_cast<QConsoleEdit *>( object );
    Console *consoleDialog = qobject_cast<Console *>( object->parent());
    if ( consoleDialog == nullptr || consoleEdit == nullptr )
        return false;

    if ( consoleEdit->hasFocus()) {
        if ( event->type() == QEvent::KeyPress ) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

            // history list -> up
            if ( keyEvent->key() == Qt::Key_Up ) {
                if ( !consoleEdit->history.isEmpty()) {
                    if ( consoleEdit->historyOffset() < consoleEdit->history.count() )
                        consoleEdit->pushHistoryOffset();

                    int offset = consoleEdit->history.count() - consoleEdit->historyOffset();

                    if ( offset > 0 )
                        consoleEdit->setText( consoleEdit->history.at( offset ));
                    else
                        consoleEdit->setText( consoleEdit->history.first());
                }
                return true;

                // history list -> down
            } else if ( keyEvent->key() == Qt::Key_Down ) {
                if ( !consoleEdit->history.isEmpty()) {
                    int offset;

                    if ( consoleEdit->historyOffset() > 0 )
                        consoleEdit->popHistoryOffset();

                    if ( consoleEdit->historyOffset() == 0 ) {
                        consoleEdit->clear();
                        return true;
                    }

                    offset = consoleEdit->history.count() - consoleEdit->historyOffset();

                    if ( offset < consoleEdit->history.count())
                        consoleEdit->setText( consoleEdit->history.at( offset ));
                    else
                        consoleEdit->setText( consoleEdit->history.last());
                }
                return true;

                // complete command
            } else if ( keyEvent->key() == Qt::Key_Tab ) {
                // abort if no text at all
                if ( consoleEdit->text().isEmpty())
                    return true;

                return consoleDialog->completeCommand();
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
    qDebug() << QObject::tr( "initilising console" );

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
