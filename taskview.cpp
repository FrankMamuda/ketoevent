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
#include "delegate.h"
#include "task.h"
#include <QDebug>

/**
 * @brief TaskView::mouseReleaseEvent
 * @param event
 */
void TaskView::mouseMoveEvent( QMouseEvent *event ) {
    if ( this->model() != nullptr && this->itemDelegate() != nullptr ) {
        Delegate *delegate( qobject_cast<Delegate*>( this->itemDelegate()));
        if ( delegate != nullptr )
            delegate->setMousePos( event->pos());
    }

    QListView::mouseMoveEvent( event );
}

/**
 * @brief TaskView::mouseReleaseEvent
 * @param event
 */
void TaskView::mouseReleaseEvent( QMouseEvent *event ) {
    if ( this->model() != nullptr && this->itemDelegate() != nullptr && event->button() == Qt::LeftButton ) {
        Delegate *delegate( qobject_cast<Delegate*>( this->itemDelegate()));
        if ( delegate != nullptr ) {
            int y;

            for ( y = 0; y < this->model()->rowCount(); y++ ) {
                const QModelIndex index( this->model()->index( y, Task::instance()->Name ));

                if ( this->visualRect( index ).contains( event->pos())) {
                    delegate->setMousePos( event->pos());
                    Item::Actions action = delegate->action( index );
                    switch ( action ) {
                    case Item::NoAction:
                        break;

                    case Item::Set:
                        // TODO:
                        //this->model()->setData( index, true, Model::Value );
                        break;

                    case Item::Edit:
                        this->edit( index );
                        break;

                    case Item::Remove:
                        // TODO:
                        //this->model()->setData( index, 0, Model::Value );
                        break;

                    case Item::Combine:
                        qDebug() << "combine";
                        break;

                    case Item::SetNumeric:
                        this->edit( index );
                        break;
                    }
                }
            }
        }
    }

    QListView::mouseReleaseEvent( event );
}

/**
 * @brief ListView::leaveEvent
 * @param event
 */
void TaskView::leaveEvent( QEvent *event ) {
    if ( this->model() != nullptr && this->itemDelegate() != nullptr ) {
        Delegate *delegate( qobject_cast<Delegate*>( this->itemDelegate()));
        if ( delegate != nullptr )
            delegate->setMousePos( QPoint(), true );
    }

    QListView::leaveEvent( event );
}
