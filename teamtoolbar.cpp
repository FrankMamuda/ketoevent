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
#include "team.h"
#include "teamtoolbar.h"
#include "teamedit.h"
#include "main.h"
#include <QDebug>
#include <QMessageBox>

// singleton
TeamToolBar *TeamToolBar::i = nullptr;

/**
 * @brief TeamToolBar::TeamToolBar
 */
TeamToolBar::TeamToolBar( QWidget *parent ) : ToolBar( parent ) {
    // add action
    this->addAction( QIcon::fromTheme( "add" ), this->tr( "Add Team" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( TeamEdit::instance(), this->tr( "Add Team " ));
            TeamEdit::instance()->reset();
        }
    } );

    // edit action
    this->edit = this->addAction( QIcon::fromTheme( "edit" ), this->tr( "Edit Team" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( TeamEdit::instance(), this->tr( "Edit Team " ));
            TeamEdit::instance()->reset( true );
        }
    } );

    // remove action
    this->remove = this->addAction( QIcon::fromTheme( "remove" ), this->tr( "Remove Team" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance()->container->currentIndex());

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid())
            return;

        const Row row = Team::instance()->row( index );
        if ( row == Row::Invalid )
            return;

        const QString title( Team::instance()->title( row ));
        const Row team = MainWindow::instance()->currentTeam();

        if ( QMessageBox::question( this, this->tr( "Remove team" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( title )) == QMessageBox::Yes )
            Team::instance()->remove( row );

        // restore teamId (model resets on remove apparently)
        if ( team != row )
            MainWindow::instance()->setCurrentTeam( team );
    } );

    // button test (disconnected in ~EditorDialog)
    this->connect( EditorDialog::instance()->container, SIGNAL( clicked( QModelIndex )), this, SLOT( buttonTest( QModelIndex )));
    this->buttonTest();

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief TeamToolBar::buttonTest
 * @param index
 */
void TeamToolBar::buttonTest( const QModelIndex &index ) {
    this->edit->setEnabled( index.isValid());
    this->remove->setEnabled( index.isValid());
};



