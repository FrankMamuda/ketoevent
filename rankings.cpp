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
#include "variable.h"
#include "main.h"
#include "database.h"
#include "log.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QTextStream>
#include <QtMath>

/**
 * @brief Rankings::Rankings
 * @param parent
 */
Rankings::Rankings() : ui( new Ui::Rankings ), model( nullptr ), proxyModel( nullptr ) {
    // set up ui
    this->ui->setupUi( this );

    // make sure this window blocks other windows
    this->setWindowModality( Qt::ApplicationModal );

    // set up progressbar and team selector
    this->ui->progressBar->hide();
    this->ui->closeButton->setIcon( QIcon( ":/icons/close" ));
    this->ui->teamCombo->setModel( Team::instance());
    this->ui->teamCombo->setModelColumn( Team::Title );

    // bind currentTeam action to a variable
    // and repaint table when either team or currentTeam variable changes
    Variable::instance()->bind( "rankingsCurrent", this->ui->actionCurrent );
    this->connect( this->ui->actionCurrent, SIGNAL( toggled( bool )), this->ui->tableView->viewport(), SLOT( repaint()));
    this->connect( this->ui->teamCombo, SIGNAL( currentIndexChanged( int )), this->ui->tableView->viewport(), SLOT( repaint()));

    // set window icon
    this->setWindowIcon( QIcon( ":/icons/rankings" ));

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief Rankings::~Rankings
 */
Rankings::~Rankings() {
    Variable::instance()->unbind( "rankingsCurrent", this->ui->actionCurrent );
    this->disconnect( this->ui->actionCurrent, SIGNAL( toggled( bool )));
    this->disconnect( this->ui->teamCombo, SIGNAL( currentIndexChanged( int )));

    delete this->ui;
}

/**
 * @brief Rankings::isDisplayingCurrentTeam
 * @return
 */
bool Rankings::isDisplayingCurrentTeam() const {
    return this->ui->actionCurrent->isChecked();
}

/**
 * @brief Rankings::on_actionUpdate_triggered
 */
void Rankings::on_actionUpdate_triggered() {
    int totalLogged = 0;

    // remove junk to make sure it does not affect results
    Log::instance()->removeOrphanedEntries();

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
    const Row event = MainWindow::instance()->currentEvent();
    if ( event == Row::Invalid )
        return;

    const QTime eventStartTime( Event::instance()->startTime( event ));
    const QTime eventFinishTime( Event::instance()->finishTime( event ));
    const QTime eventFinalTime( Event::instance()->finalTime( event ));
    const int penaltyPoints( Event::instance()->penalty( event ));

    // go through team list (might seem a little less efficient than going through logs,
    // but in reality there is not that much of a performance penalty
    // team method also avoids unnecessary complexity over calculation by logs
    for ( int team = 0; team < Team::instance()->count(); team++ ) {
        const Row teamRow = Team::instance()->row( team );

        if ( teamRow == Row::Invalid )
            continue;

        TeamStatistics stats( Team::instance()->title( teamRow ));
        QMap<Id, int> combos;
        QList<Id> dup;

        // update progress bar
        this->ui->progressBar->setValue( team );

        // go through logs
        for ( int log = 0; log < Log::instance()->count(); log++ ) {
            const Row logRow = Log::instance()->row( log );

            if ( logRow == Row::Invalid )
                continue;

            const int value = Log::instance()->multiplier( logRow );

            // abort on invalid log values or logs not related to the current team
            if ( value <= 0 || Team::instance()->id( teamRow ) != Log::instance()->teamId( logRow ))
                continue;

            // get task related variables
            const Id taskId = Log::instance()->taskId( logRow );
            const Row task = Task::instance()->row( taskId );

            if ( task == Row::Invalid )
                continue;

            const Task::Types type = Task::instance()->type( task );

            // test for duplicates
            if ( dup.contains( taskId ))
                continue;
            dup << taskId;

            // increment completed tasks
            stats.completedTasks++;

            // calculate points form completed tasks
            stats.points += Task::instance()->points( task ) * (( type == Task::Types::Multi ) ? value : 1 );

            // build combo map
            const Id comboId = Log::instance()->comboId( logRow );
            if ( !combos.contains( comboId ) && comboId > Id::Invalid )
                combos[comboId] = 0;

            if ( comboId > Id::Invalid ) {
                combos[comboId] = combos[comboId] + 1;
                stats.comboTasks += 1;
            }

#ifdef KK6_SPECIAL
            // NOTE: hardcoded
            // special event-related points
            const QString taskName( Task::instance()->name( task ));
            if ( !QString::compare( taskName, "Papilduzdevums" ))
                stats.specialPoints1 = value;
            else if ( !QString::compare( taskName, "FTF" ))
                stats.specialPoints0 = value;
#endif
        }

        // store combo count
        stats.combos = combos.count();

        // calculate bonus points from combos
        foreach ( const int count, combos ) {
            if ( count == 2 )
                stats.points += EventTable::DefaultComboOfTwo;

            if ( count == 3 )
                stats.points += EventTable::DefaultComboOfThree;

            if ( count >= 4 )
                stats.points += EventTable::DefaultComboOfFourAndMore;
        }

        // calculate penalty points
        const int overTime = eventFinishTime.secsTo( Team::instance()->finishTime( teamRow )) / 60 + 1;
        stats.time = eventStartTime.secsTo( Team::instance()->finishTime( teamRow )) / 60 + 1;
        if ( overTime > 0 ) {
            stats.penalty = penaltyPoints * overTime;
            stats.points -= stats.penalty;
            stats.points = qMax( stats.points, 0 );
        }
        const int penaltyTime = eventFinishTime.secsTo( eventFinalTime );
        if ( overTime > penaltyTime )
            stats.points = 0;

        // add team stats to list
        this->list << stats;
        totalLogged += stats.points;
    }

    // update model
    this->model->reset();

    // hide progress bar
    this->ui->progressBar->hide();

    // sort by points
    this->proxyModel->sort( RankingsModel::Points, Qt::DescendingOrder );

    // scale window to contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();

    // calculate rank
    // NOTE: a really dumb way to do it
    QMap<int,int> map;

    int y = 0;
    foreach ( const TeamStatistics &stats, this->list ) {
        map.insertMulti( stats.points, y );
        y++;
    }

    QList<int>points( map.uniqueKeys());
    std::sort( points.begin(), points.end(), std::greater<int>());
    y = 1;

    foreach ( const int p, qAsConst( points )) {
        foreach ( int index, map.values( p )) {
            TeamStatistics stats = this->list.at( index );
            stats.rank = y;
            this->list.replace( index, stats );
        }
        y++;
    }

    // total teams
    this->ui->totalTeamsEdit->setText( QString::number( Team::instance()->count()));

    // total members
    int members = 0;
    for ( int y = 0; y < Team::instance()->count(); y++ )
        members += Team::instance()->members( Team::instance()->row( y ));
    this->ui->totalMembersEdit->setText( QString::number( members ));

    // total logged points
    this->ui->totalLoggedEdit->setText( QString::number( totalLogged ));

    // total tasks
    this->ui->totalTasksEdit->setText( QString::number( Task::instance()->count()));

    // total points
    int totalPoints = 0;
    for ( int y = 0; y < Task::instance()->count(); y++ ) {
        const Row row = Task::instance()->row( y );
        const Task::Types type = Task::instance()->type( row );

        totalPoints += ( Task::instance()->points( row ) * ( type == Task::Types::Multi ? Task::instance()->multi( row ) : 1 ));
    }
    int comboPoints = static_cast<int>( qFloor( static_cast<qreal>( Task::instance()->count()) / 4 )) * Event::instance()->comboOfFourPlus( MainWindow::instance()->currentEvent());
    switch ( Task::instance()->count() % 4 ) {
    case 3:
        comboPoints += Event::instance()->comboOfThree( MainWindow::instance()->currentEvent());
        break;

    case 2:
        comboPoints += Event::instance()->comboOfTwo( MainWindow::instance()->currentEvent());
        break;
    }
    this->ui->totalPointsEdit->setText( this->tr( "%1 (%2+%3)" ).arg( totalPoints + comboPoints ).arg( totalPoints ).arg( comboPoints ));

    // # of tasks completed
    int numTasksCompleted = 0;
    QSqlQuery query;
    query.exec( QString( "SELECT COUNT(*) from %1 WHERE %2>0 GROUP BY %3" )
                .arg( Log::instance()->tableName())
                .arg( Log::instance()->fieldName( Log::Multi ))
                .arg( Log::instance()->fieldName( Log::Task )));
    while ( query.next())
        numTasksCompleted += query.value( 0 ).toInt();
    this->ui->completedEdit->setText( QString::number( numTasksCompleted ));
}

/**
 * @brief Rankings::showEvent
 * @param event
 */
void Rankings::showEvent( QShowEvent *event ) {
    ModalWindow::showEvent( event );

    // scale window to contents
    this->ui->tableView->resizeColumnsToContents();
    this->ui->tableView->resizeRowsToContents();

    // set current team
    this->ui->teamCombo->setCurrentIndex( static_cast<int>( MainWindow::instance()->currentTeam()));
}

/**
 * @brief Rankings::on_actionExport_triggered
 */
void Rankings::on_actionExport_triggered() {
    QString path( QFileDialog::getSaveFileName( this, this->tr( "Export statistics to CSV format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" )));
#ifdef Q_OS_WIN
    const bool win32 = true;
#else
    const bool win32 = false;
#endif

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
        out.setCodec( win32 ? "Windows-1257" : "UTF-8" );
        out << this->tr( "Team name;Tasks;Combos;Time;Penalty points;Total points" ).append( win32 ? "\r" : "\n" );

        foreach ( const TeamStatistics &team, this->list ) {
            out << QString( "%1;%2;%3;%4;%5;%6%7" )
                   .arg( team.title )
                   .arg( team.completedTasks )
                   .arg( team.combos )
                   .arg( team.time )
                   .arg( team.penalty )
                   .arg( team.points )
                   .arg( win32 ? "\r" : "\n" );
        }
    }
    csv.close();
}

