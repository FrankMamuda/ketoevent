/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "editordialog.h"
#include "event.h"
#include "eventtoolbar.h"
#include "eventedit.h"
#include "main.h"
#include "database.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

// singleton
EventToolBar *EventToolBar::i = nullptr;

/**
 * @brief EventToolBar::EventToolBar
 * @param parent
 */
EventToolBar::EventToolBar( QWidget *parent ) : ToolBar( parent ) {
    // add action
    this->addAction( QIcon::fromTheme( "add" ), this->tr( "Add Event" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( EventEdit::instance(), this->tr( "Add Event" ));
            EventEdit::instance()->reset();
        }
    } );

    // edit action
    this->edit = this->addAction( QIcon::fromTheme( "edit" ), this->tr( "Edit Event" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( EventEdit::instance(), this->tr( "Edit Event" ));
            EventEdit::instance()->reset( true );
        }
    } );

    // remove action
    this->remove = this->addAction( QIcon::fromTheme( "remove" ), this->tr( "Remove Event" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance()->container->currentIndex());

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid())
            return;

        const Row row = Event::instance()->row( index );
        if ( row == Row::Invalid )
            return;

        const QString title( Event::instance()->title( row ));
        const Row event = MainWindow::instance()->currentEvent();

        if ( QMessageBox::question( this, this->tr( "Remove event" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( title )) == QMessageBox::Yes ) {
            Event::instance()->remove( row );
            Database::instance()->removeOrphanedEntries();
        }

        // restore eventId (model resets on remove apparently)
        if ( event != row )
            MainWindow::instance()->setCurrentEvent( event );
    } );

    // import action
    this->addAction( QIcon::fromTheme( "export" ), this->tr( "Import logs" ), [ this ]() {
        const QFileInfo info( QFileDialog::getOpenFileName( this, this->tr( "Import logs from database" ), QDir::homePath() + "/" + "database.db", this->tr( "Database (*.db)" )));
        if ( !info.exists())
            return;

        if ( QMessageBox::question( this, this->tr( "Import event" ), this->tr( "Do you really want to import logs and teams from \"%1\"?" ).arg( info.fileName())) == QMessageBox::Yes ) {
            Database::instance()->attach( info );
        }
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
