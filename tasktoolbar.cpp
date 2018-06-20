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
#include "task.h"
#include "taskedit.h"
#include "tasktoolbar.h"
#include <QCommonStyle>
#include <QDebug>
#include <QMessageBox>

/**
 * @brief TaskToolBar::TaskToolBar
 */
TaskToolBar::TaskToolBar( QWidget *parent ) : ToolBar( parent ) {
    // add action
    this->addAction( QIcon( ":/icons/add" ), this->tr( "Add Task" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( TaskEdit::instance(), this->tr( "Add Task" ));
            TaskEdit::instance()->reset();
        }
    } );

    // edit action
    this->addAction( QIcon( ":/icons/edit" ), this->tr( "Edit Task" ), [ this ]() {
        if ( !EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock( TaskEdit::instance(), this->tr( "Edit Task" ));
            TaskEdit::instance()->reset( true );
        }
    } );

    // remove action
    QAction *remove( this->addAction( QIcon( ":/icons/remove" ), this->tr( "Remove Task" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance()->container->currentIndex());

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid())
            return;

        const QString name( Task::instance()->name( index.row()));
        if ( QMessageBox::question( this, this->tr( "Remove task" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( name )) == QMessageBox::Yes )
            Task::instance()->remove( index.row());
    } ));
    remove->setEnabled( false );

    // move up action
    QAction *moveUp( this->addAction( QIcon( ":/icons/up" ), this->tr( "Move up" ), [ this ]() {
    } ));
    moveUp->setEnabled( false );

    // move down action
    QAction *moveDown( this->addAction( QIcon( ":/icons/down" ), this->tr( "Move down" ), [ this ]() {
    } ));
    moveDown->setEnabled( false );

    // button test
    this->connect( EditorDialog::instance()->container, &QListView::clicked, [ this, remove, moveUp, moveDown ]( const QModelIndex &index ) {
        remove->setEnabled( index.isValid());
        moveUp->setEnabled( index.isValid() && index.row() != 0 );
        moveDown->setEnabled( index.isValid() && index.row() != Task::instance()->count() - 1 );
    } );
}
