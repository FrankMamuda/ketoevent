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
#include "taskview.h"
#include "delegate.h"
#include "task.h"
#include "mainwindow.h"
#include "log.h"
#include "team.h"
#include "popup.h"
#include "database.h"
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

                // comboId lambda
                auto setComboId = [index, delegate]( const Id &id ) {
                    const Row task = delegate->row( index );
                    const Row team = MainWindow::instance()->currentTeam();

                    if ( task == Row::Invalid || team == Row::Invalid )
                        return;

                    const Id taskId = Task::instance()->id( task );
                    const Id teamId = Team::instance()->id( team );

                    const Id logId = Log::instance()->id( taskId, teamId );
                    if ( logId == Id::Invalid )
                        return;

                    const Row log = Log::instance()->row( logId );
                    if ( log == Row::Invalid )
                        return;

                    Log::instance()->setComboId( log, id );

                    // increment counter
                    Database::instance()->incrementCounter();
                };

                if ( this->visualRect( index ).contains( event->pos())) {
                    delegate->setMousePos( event->pos());
                    Item::Actions action = delegate->action( index );
                    switch ( action ) {
                    case Item::NoAction:
                        break;

                    case Item::Set:
                        Task::instance()->setMultiplier( delegate->row( index ), true );

                        // increment counter
                        Database::instance()->incrementCounter();

                        break;

                    case Item::Edit:
                        this->edit( index );
                        break;

                    case Item::Remove:
                        Task::instance()->setMultiplier( delegate->row( index ), false );

                        // increment counter
                        Database::instance()->incrementCounter();
                        break;


                    case Item::InfoPopup:
                    {
                        const QString desc( Task::instance()->description( Task::instance()->row( index )));
                        if ( desc.isEmpty())
                            break;

                        Popup( MainWindow::instance(), desc ).exec();
                    }
                        break;

                    case Item::Combine:
                        if ( MainWindow::instance()->isComboModeActive()) {
                            MainWindow::instance()->setTaskFilter();
                            delegate->reset();
                        } else {
                            // TODO: restore position in list after filtering
                            Id id = static_cast<Id>( delegate->combos[index] );
                            if ( id == Id::Invalid ) {
                                QSqlQuery query;
                                query.exec( QString( "select max( %1 ) from %2" )
                                            .arg( Log::instance()->fieldName( Log::Combo ),
                                                  Log::instance()->tableName()));

                                id = query.next() ? static_cast<Id>( query.value( 0 ).toInt() + 1 ) : static_cast<Id>( 0 );
                            }

#ifdef QT_DEBUG
                            qCDebug( Database_::Debug ) << "combine";
#endif

                            QSqlQuery query;
                            int count = 0;
                            query.exec( QString( "select COUNT(*) from logs where logs.value>0 and ( comboId=%1 or comboId=-1) and logs.teamId=%2" )
                                        .arg( static_cast<int>( id ))
                                        .arg( static_cast<int>( Team::instance()->id( MainWindow::instance()->currentTeam()))));
                            if ( query.next())
                                count = query.value( 0 ).toInt();

#ifdef QT_DEBUG
                            qCDebug( Database_::Debug ) << "found" << count;
#endif

                            // disallow combination if only one valid task is available
                            if ( count <= 1 )
                                return;

                            setComboId( id );

                            // required to reset id list accessed by "done" action
                            delegate->reset();

#ifdef QT_DEBUG
                            qCDebug( Database_::Debug ) << "set id" << id;
#endif

                            MainWindow::instance()->setTaskFilter( true, id );
                        }
                        break;

                    case Item::SetNumeric:
                        this->edit( index );

                        // increment counter
                        Database::instance()->incrementCounter();
                        break;

                    case Item::AddCombo:
#ifdef QT_DEBUG
                        qCDebug( Database_::Debug ) << "add combo";
#endif

                        setComboId( MainWindow::instance()->currentComboId());
                        break;

                    case Item::RemoveCombo:
                        setComboId( Id::Invalid );
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

