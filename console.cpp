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
#include "cmd.h"
#include "console.h"
#include "ui_console.h"
#include "variable.h"
#include "main.h"

#ifdef QT_DEBUG

/**
 * @brief Console::Console
 */
Console::Console() : ui( new Ui::Console ) {
    this->ui->setupUi( this );
    this->edit = this->ui->input;
    this->setWindowFlags( Qt::Tool );
    this->setWindowOpacity( 0.95 );
    this->ui->screen->clear();

    // install event filter
    this->edit->installEventFilter( this );
    this->edit->history = Variable::instance()->string( "system/consoleHistory" ).split( ";" );

    // set window icon
    this->setWindowIcon( QIcon( ":/icons/console" ));

    // announce
    this->print( this->tr( "This is the console. Type 'help' if unsure what to do.\n" ));
}

/**
 * @brief Console::~Console
 */
Console::~Console() {
    Variable::instance()->setValue( "system/consoleHistory", this->edit->history.join( ";" ));
    this->edit->removeEventFilter( this );
    delete this->ui;
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
    foreach ( const QString &name, Cmd::instance()->keys()) {
        if ( name.startsWith( this->edit->text()))
            matchedStrings << name;
    }

    // find matching cvars
    foreach ( const QSharedPointer<Var> &entry, Variable::instance()->list ) {
        if ( !QString::compare( entry->key(), "system/consoleHistory" ))
            continue;

        if ( entry->key().startsWith( this->edit->text()))
            matchedStrings << entry->key();
    }

    // complete to shortest string
    if ( matchedStrings.count() == 1 ) {
        // append extra space (since it's the only match that will likely be follwed by an argument)
        this->edit->setText( matchedStrings.first() + " " );
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
        this->edit->setText( matchedStrings.first().left( match ));
    } else if ( !matchedStrings.count()) {
        return true;
    }

    // print out suggestions
    qInfo() << this->tr( "Available commands and cvars:" );
    foreach ( const QString &str, matchedStrings ) {
        // check commands
        if ( Cmd::instance()->keys().contains( str )) {
            QString description( Cmd::instance()->description( str ));
            qInfo() << ( !description.isEmpty() ? QString( "  \"%1\" - %2" ).arg( str, description ) : QString( "  \"%1" ).arg( str ));
        }

        // check variables
        if ( Variable::instance()->contains( str )) {
            QSharedPointer<Var> entry( Variable::instance()->list[str] );
            qInfo() << this->tr( "  \"%1\" is \"%2\"" ).arg( entry->key(), entry->value().toString());
        }
    }

    // add extra newline
    qInfo() << "\n";
    return true;
}

/**
 * @brief ConsoleEventFilter::eventFilter
 * @param object
 * @param event
 * @return
 */
bool Console::eventFilter( QObject *object, QEvent *event ) {
    HistoryEdit *edit( qobject_cast<HistoryEdit *>( object ));

    if ( edit == nullptr )
        return false;

    if ( edit->hasFocus()) {
        if ( event->type() == QEvent::KeyPress ) {
            QKeyEvent *keyEvent( static_cast<QKeyEvent*>( event ));

            if ( keyEvent->key() == Qt::Key_Up ) {
                if ( !edit->history.isEmpty()) {
                    if ( edit->offset() < edit->history.count())
                        edit->push();

                    const int offset = edit->history.count() - edit->offset();
                    edit->setText( offset > 0 ? edit->history.at( offset ) : edit->history.first());
                }
                return true;
            } else if ( keyEvent->key() == Qt::Key_Down ) {
                if ( !edit->history.isEmpty()) {
                    if ( edit->offset() > 0 )
                        edit->pop();

                    if ( edit->offset() == 0 ) {
                        edit->clear();
                        return true;
                    }

                    const int offset = edit->history.count() - edit->offset();
                    edit->setText( offset < edit->history.count() ? edit->history.at( offset ) : edit->history.last());
                }
                return true;
            } else if ( keyEvent->key() == Qt::Key_Tab ) {
                if ( edit->text().isEmpty())
                    return true;

                return this->completeCommand();
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
    QString out( msg );

    if ( out.startsWith( '"' ))
        out = out.mid( 1, out.length() - 2 );

    this->ui->screen->append( out.replace( "\\\"", "\"" ) );

    // move cursor
    QTextCursor cursor( this->ui->screen->textCursor());
    cursor.movePosition( QTextCursor::End );
    this->ui->screen->setTextCursor( cursor );
}

/**
 * @brief Console::on_input_returnPressed
 */
void Console::on_input_returnPressed() {
    if ( Cmd::instance()->execute( this->edit->text()))
        this->edit->add( this->edit->text());

    // set min offset
    this->edit->reset();
    this->edit->clear();
}

#endif
