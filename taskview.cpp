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
#include "mainwindow.h"
#include "log.h"
#include <QDebug>
#include <QSqlQuery>

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
                        Task::instance()->setMultiplier( delegate->proxy( index ).row(), true );
                        break;

                    case Item::Edit:
                        this->edit( index );
                        break;

                    case Item::Remove:
                        Task::instance()->setMultiplier( delegate->proxy( index ).row(), false );
                        break;

                    case Item::Combine:
                        if ( MainWindow::instance()->isComboModeActive())
                            MainWindow::instance()->setTaskFilter();
                        else {
                            // FIXME: if invalid, add a new one (not combined)
                            // FIXME: quit if no combos selected
                            // FIXME: disallow single log combo
                            // TODO: better sorting of combos (logged first)
                            // TODO: restore position in list after filtering
                            Id id = static_cast<Id>( delegate->combos[index] );
                            if ( id == Id::Invalid ) {
                                QSqlQuery query;
                                query.exec( "select max( comboId ) from logs" );
                                if ( query.next()) {
                                    id = static_cast<Id>( query.value( 0 ).toInt() + 1 );
                                } else {
                                    id = static_cast<Id>( 0 );
                                }
                            }

                            const QList<Id> ids( Log::instance()->ids( Task::instance()->id( delegate->proxy( index ).row()), MainWindow::instance()->currentTeamId()));
                            if ( ids.first() != Id::Invalid )
                                Log::instance()->setComboId( Log::instance()->row( ids.first()), id );

                            MainWindow::instance()->setTaskFilter( true, id );
                        }
                        break;

                    case Item::SetNumeric:
                        this->edit( index );
                        break;

                    case Item::AddCombo:
                    {
                        const QList<Id> ids( Log::instance()->ids( Task::instance()->id( delegate->proxy( index ).row()), MainWindow::instance()->currentTeamId()));
                        if ( ids.first() != Id::Invalid )
                            Log::instance()->setComboId( Log::instance()->row( ids.first()), MainWindow::instance()->currentComboId());
                        //qDebug() << "ADD COMBO for" << Task::instance()->name( delegate->proxy( index ).row()) << "with comboId" << (int )MainWindow::instance()->currentComboId();

                        // TODO: modify values inside model cache, not force a reset
                        delegate->reset();
                    }
                        break;

                    case Item::RemoveCombo:
                        const QList<Id> ids( Log::instance()->ids( Task::instance()->id( delegate->proxy( index ).row()), MainWindow::instance()->currentTeamId()));
                        if ( ids.first() != Id::Invalid )
                            Log::instance()->setComboId( Log::instance()->row( ids.first()), Id::Invalid );

                        delegate->reset();
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
            delegate->setMousePos( QPoint( this->rect().x() - 1, 0 ), true );
    }

    QListView::leaveEvent( event );
}
