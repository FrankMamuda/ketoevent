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
#include "event.h"
#include "rankingsmodel.h"
#include <QCommonStyle>
#include <QFileDialog>
#include <QTextStream>
#include <QThread>

/**
 * @brief Rankings::Rankings
 * @param parent
 */
Rankings::Rankings() : ui( new Ui::Rankings ), model( nullptr ), proxyModel( nullptr ) {
    QCommonStyle style;
    this->ui->setupUi( this );
    this->setWindowModality( Qt::ApplicationModal );
    this->ui->progressBar->hide();
    this->ui->closeButton->setIcon( style.standardIcon( QStyle::SP_DialogCloseButton ));
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
    // set up and show progress bar
    this->ui->progressBar->setRange( 0, Team::instance()->count());
    this->ui->progressBar->setValue( 0 );
    this->ui->progressBar->show();

    // create model if none available
    if ( this->model == nullptr || this->proxyModel == nullptr ) {
        this->model = new RankingsModel();
        this->proxyModel = new QSortFilterProxyModel();
        this->proxyModel->setSourceModel( this->model );
        this->proxyModel->setDynamicSortFilter( true );

        // setup table
        this->ui->tableView->setModel( proxyModel );
        this->ui->tableView->setWordWrap( true );
        this->ui->tableView->verticalHeader()->hide();
        this->ui->tableView->setSortingEnabled( true );
    }

    // clear any leftover results
    this->list.clear();
    this->model->reset();

    // get event related variables
    const int event = Event::instance()->row( MainWindow::instance()->currentEventId());
    const QTime eventStartTime( Event::instance()->startTime( event ));
    const QTime eventFinishTime( Event::instance()->finishTime( event ));
    const int penaltyPoints( Event::instance()->penalty( event ));

    // go through team list (might seem a little less efficient than going through logs,
    // but in reality there is not that much of a performance penalty
    // team method also avoids unnecessary complexity over calculation by logs
    for ( int team = 0; team < Team::instance()->count(); team++ ) {
        TeamStatistics stats( Team::instance()->title( team ));
        QMap<Id, int> combos;
        QList<Id> dup;

        // update progress bar
        this->ui->progressBar->setValue( team );

        // TODO: remove me
        QObject().thread()->usleep( 1000 * 20 );

        // go through logs
        for ( int log = 0; log < Log::instance()->count(); log++ ) {
            const int value = Log::instance()->multiplier( log );

            // abort on invalid log values or logs not related to the current team
            if ( value <= 0 || team != Log::instance()->team( log ))
                continue;

            // get task related variables
            const Id taskId = Log::instance()->taskId( log );
            const int task = Task::instance()->row( taskId );
            const Task::Types type = Task::instance()->type( task );

            // test for duplicates
            if ( dup.contains( taskId ))
                continue;
            dup << taskId;

            // increment completed tasks
            stats.completedTasks++;

            // calculate points form completed tasks
            stats.points += Task::instance()->points( task ) * (( type == Task::Multi ) ? value : 1 );

            // build combo map
            const Id comboId = Log::instance()->comboId( log );
            if ( !combos.contains( comboId ) && comboId.value() >= 0 )
                combos[comboId] = 0;

            if ( comboId.value() >= 0 ) {
                combos[comboId] = combos[comboId] + 1;
                stats.comboTasks += 1;
            }
        }

        // store combo count
        stats.combos = combos.count();

        // calculate bonus points from combos
        foreach ( int count, combos ) {
            if ( count == 2 )
                stats.points += EventTable::DefaultComboOfTwo;

            if ( count == 3 )
                stats.points += EventTable::DefaultComboOfThree;

            if ( count >= 4 )
                stats.points += EventTable::DefaultComboOfFourAndMore;
        }


        // calculate penalty points
        // TODO: check finalTime
        const int overTime = eventFinishTime.secsTo( Team::instance()->finishTime( team )) / 60 + 1;
        stats.time = eventStartTime.secsTo( Team::instance()->finishTime( team )) / 60 + 1;
        if ( overTime > 0 ) {
            stats.penalty = penaltyPoints * overTime;
            stats.points -= stats.penalty;
        }

        // add team stats to list
        this->list << stats;
    }

    // update model
    this->model->reset();

    // hide progress bar
    this->ui->progressBar->hide();

    // sort by points
    this->proxyModel->sort( 5, Qt::DescendingOrder );

    // scale window to contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();
}

/**
 * @brief Rankings::showEvent
 * @param event
 */
void Rankings::showEvent( QShowEvent *event ) {
    this->prevFilter = Log::instance()->filter();
    Log::instance()->setFilter( "" );
    ModalWindow::showEvent( event );

    // calculate results on first open
   // if ( this->model == nullptr )
        //this->on_actionUpdate_triggered();
}

/**
 * @brief Rankings::hideEvent
 * @param event
 */
void Rankings::hideEvent( QHideEvent *event ) {
    Log::instance()->setFilter( this->prevFilter );
    ModalWindow::hideEvent( event );
}

/**
 * @brief Rankings::on_closeButton_clicked
 */
void Rankings::on_closeButton_clicked() {
    this->hide();
}

/**
 * @brief Rankings::on_actionExport_triggered
 */
void Rankings::on_actionExport_triggered() {
    QString path( QFileDialog::getSaveFileName( this, this->tr( "Export statistics to CSV format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" )));

    // check for empty filenames
    if ( path.isEmpty())
        return;

    // add extension
    if ( !path.endsWith( ".csv" ))
        path.append( ".csv" );

    // create file
    QFile csv( path );

    if ( csv.open( QFile::WriteOnly | QFile::Truncate )) {
        QTextStream out( &csv );
#ifdef Q_OS_WIN
        out.setCodec( "Windows-1257" );
#else
        out.setCodec( "UTF-8" );
#endif
        out << this->tr( "Team name;Tasks;Combos;Time;Penalty points;Total points" )
       #ifdef Q_OS_WIN
               .append( "\r" )
       #endif
               .append( "\n" );

        foreach ( TeamStatistics team, this->list ) {
            int points;

            //if ( team->disqualified())
            //    points = 0;
            //else
            //    points = team->points() - team->penalty();

            // TODO: add this to calculation
            if ( points <= 0 )
                points = 0;

            out << QString( "%1;%2;%3;%4;%5;%6%7" )
                   .arg( team.title )
                   .arg( team.completedTasks )
                   .arg( team.combos )
                   .arg( team.time )
                   .arg( team.penalty )
                   .arg( team.points )
       #ifdef Q_OS_WIN
                   .arg( "\r\n" );
#else
                   .arg( "\n" );
#endif
        }
    }
    csv.close();
}
