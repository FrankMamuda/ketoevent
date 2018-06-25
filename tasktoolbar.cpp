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
#include "main.h"
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
    this->remove = this->addAction( QIcon( ":/icons/remove" ), this->tr( "Remove Task" ), [ this ]() {
        const QModelIndex index( EditorDialog::instance()->container->currentIndex());

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid())
            return;

        const QString name( Task::instance()->name( index.row()));
        if ( QMessageBox::question( this, this->tr( "Remove task" ), this->tr( "Do you really want to remove \"%1\"?" ).arg( name )) == QMessageBox::Yes )
            Task::instance()->remove( index.row());
    } );
    this->remove->setEnabled( false );

    // button test lambda
    auto buttonTest = [ this ]( const QModelIndex &index ) {
        this->remove->setEnabled( index.isValid());
        this->moveUp->setEnabled( index.isValid() && index.row() != 0 );
        this->moveDown->setEnabled( index.isValid() && index.row() != Task::instance()->count() - 1 );
    };

    // move up/down lambda
    auto move = [ this, buttonTest ]( bool up ) {
        QListView *container( EditorDialog::instance()->container );
        const QModelIndex index( container->currentIndex());
        const QModelIndex other( container->model()->index( container->currentIndex().row() + ( up ? -1 : 1 ), 0 ));

        if ( EditorDialog::instance()->isDockVisible() || !index.isValid() || !other.isValid())
            return;

        // use ids in lookup
        const Id id0 = Task::instance()->id( index.row());
        const Id id1 = Task::instance()->id( other.row());
        const int order0 = Task::instance()->order( index.row());
        const int order1 = Task::instance()->order( other.row());

        // swap order
        Task::instance()->setOrder( Task::instance()->row( id0 ), order1 );
        Task::instance()->setOrder( Task::instance()->row( id1 ), order0 );

        Task::instance()->select();
        const QModelIndex current( container->model()->index( Task::instance()->row( id0 ), 0 ));
        container->setCurrentIndex( current );
        container->setFocus();
        buttonTest( current );
    };

    // move up action
    this->moveUp = this->addAction( QIcon( ":/icons/up" ), this->tr( "Move up" ), [ move ]() {
        move( true );
    } );
    moveUp->setEnabled( false );

    // move down action
    this->moveDown = this->addAction( QIcon( ":/icons/down" ), this->tr( "Move down" ), [ move ]() {
        move( false );
    } );
    moveDown->setEnabled( false );

    // button test
    this->connect( EditorDialog::instance()->container, &QListView::clicked, [ buttonTest ]( const QModelIndex &index ) {
        buttonTest( index );
    } );

    // add to garbage man
    //GarbageMan::instance()->add( this );
}
