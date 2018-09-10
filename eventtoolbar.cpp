/*
 * Copyright (C) 2018 Factory #12
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
#include "editordialog.h"
#include "event.h"
#include "eventtoolbar.h"
#include "eventedit.h"
#include "main.h"
#include <QDebug>
#include <QMessageBox>

/**
 * @brief EventToolBar::EventToolBar
 * @param parent
 */
EventToolBar::EventToolBar( QWidget *parent ) : ToolBar( parent ) {
    // add action
    this->addAction( QIcon( ":/icons/add" ), this->tr( "Add Event" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( EventEdit::instance(), this->tr( "Add Event" ));
            EventEdit::instance()->reset();
        }
    } );

    // edit action
    this->edit = this->addAction( QIcon( ":/icons/edit" ), this->tr( "Edit Event" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( EventEdit::instance(), this->tr( "Edit Event" ));
            EventEdit::instance()->reset( true );
        }
    } );

    // remove action
    this->remove = this->addAction( QIcon( ":/icons/remove" ), this->tr( "Remove Event" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance()->container->currentIndex());

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid())
            return;

        const Row row = Event::instance()->indexToRow( index );
        if ( row == Row::Invalid )
            return;

        const QString title( Event::instance()->title( row ));
        const Id eventId = MainWindow::instance()->currentEventId();
        const Id removeId = Event::instance()->id( row );

        if ( QMessageBox::question( this, this->tr( "Remove event" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( title )) == QMessageBox::Yes )
            Event::instance()->remove( row );

        // restore eventId (model resets on remove apparently)
        // TODO:
        // if ( eventId != removeId )
        //    MainWindow::instance()->setCurrentE( eventId );
    } );

    // button test (disconnected in ~EditorDialog)
    this->connect( EditorDialog::instance()->container, SIGNAL( clicked( QModelIndex )), this, SLOT( buttonTest( QModelIndex )));
    this->buttonTest();

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief EventToolBar::buttonTest
 * @param index
 */
void EventToolBar::buttonTest( const QModelIndex &index ) {
    this->edit->setEnabled( index.isValid());
    this->remove->setEnabled( index.isValid());
};
