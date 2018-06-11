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
#include "taskview.h"
#include <QDebug>

/**
 * @brief TaskView::TaskView
 * @param parent
 */
TaskView::TaskView( QWidget *parent ) : QListView( parent ) {
    this->setMouseTracking( true );
}

/**
 * @brief TaskView::mouseMoveEvent
 * @param event
 */
void TaskView::mouseMoveEvent( QMouseEvent *event ) {
    QModelIndex current( this->indexAt( this->viewport()->mapFromGlobal( QCursor::pos())));
    QModelIndex under( current );
    QPoint pos( this->viewport()->mapToGlobal( this->visualRect( current ).topRight()));

    if ( !QRect( pos.x() - 32, pos.y(), 32, 32 ).contains( QCursor::pos()))
        under = QModelIndex();

    if ( this->check != under ) {
        this->check = under;
        this->update( current );
    }

    QListView::mouseMoveEvent( event );
}
