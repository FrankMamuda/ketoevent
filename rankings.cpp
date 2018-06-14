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
#include "rankings.h"
#include "ui_rankings.h"
#include "mainwindow.h"
#include "team.h"
#include "log.h"
#include "task.h"
#include <QtConcurrent>
#include "event.h"
#include "rankingsmodel.h"

/**
 * @brief Rankings::Rankings
 * @param parent
 */
Rankings::Rankings() : ui( new Ui::Rankings ), model( nullptr ) {
    this->ui->setupUi( this );
    this->setWindowModality( Qt::ApplicationModal );
    this->progress.setLabelText( this->tr( "Recalculating stats..." ));
}

/**
 * @brief Rankings::~Rankings
 */
Rankings::~Rankings() {
    delete this->ui;
}

/**
 * @brief Rankings::on_actionUpdate_triggered
 */
void Rankings::on_actionUpdate_triggered() {
    this->progress.setRange( 0, Team::instance()->count());
    this->progress.setValue( 0 );
    this->progress.show();

    if ( this->model == nullptr ) {
        this->model = new RankingsModel();

        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();
        proxyModel->setSourceModel( this->model );
        proxyModel->setDynamicSortFilter( true );

        this->ui->tableView->setModel( proxyModel );
        this->ui->tableView->setWordWrap( true );
        this->ui->tableView->verticalHeader()->hide();
        this->ui->tableView->setSortingEnabled( true );

    }


    this->model->list.clear();

    int team, log;
    for ( team = 0; team < Team::instance()->count(); team++ ) {
        this->progress.setValue( team );
        TeamStatistics stats( Team::instance()->title( team ));
        QMap<int,int>combos;

        QList<int> dup;

        for ( log = 0; log < Log::instance()->count(); log++ ) {
            const int value = Log::instance()->multiplier( log );

            if ( value <= 0 )
                continue;

            if ( team == Log::instance()->row( Log::instance()->teamId( log ))) {
                const int taskId = Log::instance()->taskId( log );
                const int task = Task::instance()->row( taskId );
                Task::Types type = Task::instance()->type( task );

                if ( dup.contains( taskId ))
                    continue;

                if ( !QString::compare( Task::instance()->name( task ), "Papilduzdevums" ))
                    stats.extra = value;

                dup << taskId;

                stats.completedTasks++;

                if ( type == Task::Check )
                    stats.points += Task::instance()->points( task );
                else if ( type == Task::Multi )
                    stats.points += Task::instance()->points( task ) * value;

                const int comboId = Log::instance()->comboId( log );
                if ( !combos.contains( comboId ) && comboId >= 0 )
                    combos[comboId] = 0;

                if ( comboId >= 0 ) {
                    combos[comboId] = combos[comboId] + 1;
                    stats.comboTasks += 1;
                }
            }
        }

        stats.combos = combos.count();
        foreach ( int key, combos ) {
            if ( combos.count( key ) == 2 )
                stats.points += EventTable::DefaultComboOfTwo;

            if ( combos.count( key ) == 3 )
                stats.points += EventTable::DefaultComboOfThree;

            if ( combos.count( key ) >= 4 )
                stats.points += EventTable::DefaultComboOfFourAndMore;
        } //qDebug() << Team::instance()->title( team ) << combos.count();

        this->model->list << stats;
    }

    this->model->reset();
    this->progress.hide();
}

/**
 * @brief Rankings::showEvent
 * @param event
 */
void Rankings::showEvent( QShowEvent *event ) {
    this->prevFilter = Log::instance()->filter();
    Log::instance()->setFilter( "" );
    ModalWindow::showEvent( event );
}

/**
 * @brief Rankings::hideEvent
 * @param event
 */
void Rankings::hideEvent( QHideEvent *event ) {
    Log::instance()->setFilter( this->prevFilter );
    ModalWindow::hideEvent( event );
}
