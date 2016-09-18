/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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
#include "gui_rankings.h"
#include "ui_gui_rankings.h"
#include <QFileDialog>
#include <QTextStream>
#include "main.h"
#include "gui_main.h"
#ifdef APPLET_DEBUG
#include <QListView>
#include <QHBoxLayout>
#endif

/**
 * @brief Gui_Rankings::Gui_Rankings Team ranking and event statistics display (constructor)
 * @param parent parent widget
 */
Gui_Rankings::Gui_Rankings( QWidget *parent ) : Gui_Dialog( parent ), ui( new Ui::Gui_Rankings ) {
    // set up ui
    ui->setupUi( this );

    // failsafe
    if ( m.isInitialised())
        this->bindVars();
    else
        this->onRejected();

    // define column headers here for now (required for proper translations)
    this->columnHeaders <<
                           this->tr( "Rank" ) <<
                           this->tr( "Team name" ) <<
                           this->tr( "Tasks" ) <<
                           this->tr( "Combos" ) <<
                           this->tr( "Combined\ntasks" ) <<
                           this->tr( "Time\n(min)" ) <<
                           this->tr( "Penalty\npoints" ) <<
                           this->tr( "Reviewer" ) <<
                           this->tr( "Total\npoints" );

    // set up ranking view
    this->ui->rankingView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    this->ui->rankingView->setSortingEnabled( true );

    // set up sorting & view model
    this->proxyModel = new RankingsSortModel( this );
    this->modelPtr = new QStandardItemModel( m.currentEvent()->teamList.count(), NumRankingColumns, this );
    this->proxyModel->setSourceModel( this->modelPtr );
    this->proxyModel->setDynamicSortFilter( true );
    this->ui->rankingView->verticalHeader()->hide();
    this->ui->rankingView->setModel( this->proxyModel );
    this->ui->rankingView->setWordWrap( true );

    // connect
    this->connect( m.cvar( "rankings/current" ), SIGNAL( changed()), this, SLOT( currentTeamToggled()));

    // set stats
    this->calculateStatistics();
    this->fillHeader();
    this->fillData();
    this->proxyModel->sort( Points, Qt::DescendingOrder );

    // scale window to contents
    this->ui->rankingView->resizeColumnsToContents();
    this->ui->rankingView->resizeRowsToContents();
    this->rescaleWindow();

    // disable horizontal scrollbar
    this->ui->rankingView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    // set focus
    this->ui->closeButton->setFocus();

#ifndef APPLET_DEBUG
    this->ui->toolBar->removeAction( this->ui->actionNoLogs );
#endif
}

/**
 * @brief greaterThan quick integer compare for sorting purposes
 * @param a first integer
 * @param b second integer
 * @return
 */
static bool greaterThan( int a, int b ) {
    return a > b;
}

/**
 * @brief Gui_Rankings::clearData clears stats from rankingView
 */
void Gui_Rankings::clearData() {
    this->ui->rankingView->model()->removeRows( 0, this->ui->rankingView->model()->rowCount());
}

/**
 * @brief Gui_Rankings::fillHeader feeds header data to rankingView (team name, points, etc.)
 */
void Gui_Rankings::fillHeader() {
    QFont boldFont;
    unsigned int y;
    boldFont.setBold( true );

    // generate header
    for ( y = 0; y < NumRankingColumns; y++ ) {
        QStandardItem *itemPtr = new QStandardItem( this->columnHeaders.at( y ));
        itemPtr->setFont( boldFont );
        itemPtr->setData( Qt::AlignCenter, Qt::TextAlignmentRole );
        this->modelPtr->setHorizontalHeaderItem( y, itemPtr );
    }
}

/**
 * @brief Gui_Rankings::currentTeamToggled refreshes view (hiding other teams, if toggled)
 */
void Gui_Rankings::currentTeamToggled() {
    this->clearData();
    this->fillData();
}

/**
 * @brief Gui_Rankings::fillData feeds data to rankingView
 */
void Gui_Rankings::fillData() {
    unsigned int y;
    int k, points;
    QList<int> pointsList;

    // fill data
    for ( y = 0; y < NumRankingColumns; y++ ) {
        for ( k = 0; k < m.currentEvent()->teamList.count(); k++ ) {
            Team *teamPtr = m.currentEvent()->teamList.at( k );
            Gui_Main *guiPtr = qobject_cast<Gui_Main*>( this->parent());
            QStandardItem *itemPtr = new QStandardItem();
            QString text;

            if ( teamPtr == NULL || guiPtr == NULL )
                return;

            switch ( y ) {
            case TeamName:
                text = teamPtr->name();

                if ( m.cvar( "rankings/current" )->isEnabled()) {
                    if ( teamPtr != m.teamForId( guiPtr->currentTeamId()))
                        text = "";
                }
                break;

            case Tasks:
                text = QString( "%1" ).arg( teamPtr->logList.count());
                break;

            case Combos:
                text = QString( "%1" ).arg( teamPtr->combos());
                break;

            case Total:
                text = QString( "%1" ).arg( teamPtr->total());
                break;

            case Time:
                text = QString( "%1" ).arg( teamPtr->timeOnTrack());
                break;

            case Reviewer:
                text = teamPtr->reviewer();
                break;

            case Penalty:
                if ( teamPtr->penalty() > 0 )
                    itemPtr->setForeground( QColor( Qt::red ));
                text = QString( "%1" ).arg( teamPtr->penalty());
                break;

            case Points:
                if ( teamPtr->disqualified())
                    text = QString( "-1" );

                points = teamPtr->points() - teamPtr->penalty();

                if ( points <= 0 )
                    points = 0;

                // avoid duplicates
                if ( pointsList.indexOf( points ) == -1 )
                    pointsList << points;

                text = QString( "%1" ).arg( points );
                break;

            default:
                break;
            }

            // disqualified
            if ( teamPtr->disqualified())
                itemPtr->setForeground( QColor( Qt::red ));

            // make it centred
            itemPtr->setData( Qt::AlignCenter, Qt::TextAlignmentRole );
            itemPtr->setText( text );
            this->modelPtr->setItem( k, y, itemPtr );
        }
    }

    // sort by points
    qSort( pointsList.begin(), pointsList.end(), greaterThan );

    // second pass for rank
    for ( k = 0; k < m.currentEvent()->teamList.count(); k++ ) {
        int rankColumn = Rank;
        int pointsColumn = Points;
        this->modelPtr->item( k, rankColumn )->setText( QString( "%1" ).arg( pointsList.indexOf( this->modelPtr->item( k, pointsColumn )->text().toInt()) + 1 ));
    }
}

/**
 * @brief Gui_Rankings::calculateStatistics recalculates event stats
 */
void Gui_Rankings::calculateStatistics() {
    int numParcipiants = 0, numTasks = 0, pointsLogged = 0, maxPoints = 0;

    // get team stats
    foreach ( Team *teamPtr, m.currentEvent()->teamList ) {
        numParcipiants += teamPtr->members();
        numTasks += teamPtr->logList.count();
        teamPtr->setCombosCalculated( false );
        teamPtr->calculateCombos();
        pointsLogged += teamPtr->points();
    }

    // get max points
    foreach ( Task *taskPtr, m.currentEvent()->taskList ) {
        if ( taskPtr->type() == Task::Check )
            maxPoints += taskPtr->points();
        else if ( taskPtr->type() == Task::Multi )
            maxPoints += taskPtr->points() * taskPtr->multi();
    }

    // display data
    this->ui->tTeams->setText( QString( "%1\n" ).arg( m.currentEvent()->teamList.count()));
    this->ui->tPar->setText( QString( "%1\n" ).arg( numParcipiants ));
    this->ui->tTasks->setText( QString( "%1\n" ).arg( numTasks ));
    this->ui->tTasksTotal->setText( QString( "%1\n" ).arg( m.currentEvent()->taskList.count()));
    this->ui->tPoints->setText( QString( "%1\n" ).arg( maxPoints ));
    this->ui->tPointsLogged->setText( QString( "%1\n" ).arg( pointsLogged ));
}

/**
 * @brief Gui_Rankings::rescaleWindow recalculates window geometry to minimum
 */
void Gui_Rankings::rescaleWindow() {
    QRect tableRect, windowRect, parentRect;
    int y, columnWidths = 0;

    // scale window to contents
    tableRect = this->ui->rankingView->geometry();
    windowRect = this->geometry();
    parentRect = this->parentWidget()->geometry();

    for ( y = 0; y < static_cast<int>( NumRankingColumns ); y++ )
        columnWidths += this->ui->rankingView->columnWidth( y );

    // this looks fairly fine
    tableRect.setWidth( 2 + this->ui->rankingView->verticalHeader()->width() + columnWidths );
    this->ui->rankingView->setGeometry( tableRect );
    windowRect = this->geometry();
    windowRect.setWidth( 24 + tableRect.width() + this->ui->layoutTeamRankings->margin() * 2 + this->ui->groupRankings->layout()->margin() * 2 );
    this->setFixedWidth( windowRect.width());
}

/**
 * @brief Gui_Rankings::~Gui_Rankings destructor
 */
Gui_Rankings::~Gui_Rankings() {
    this->clearData();

    delete ui;
    this->unbindVars();

    // this should trigger deletion of QStandardItems
    this->modelPtr->clear();

    // delete sort and data models
    delete this->modelPtr;
    delete this->proxyModel;
    this->columnHeaders.clear();
}

/**
 * @brief Gui_Rankings::bindVars connects console/settings variables for updates
 */
void Gui_Rankings::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    this->bindVariable( "rankings/current", this->ui->actionCurrentTeam );

    // unlock vars
    this->lockVariables( false );
}

/**
 * @brief Gui_Rankings::on_closeButton_clicked closes rankings display
 */
void Gui_Rankings::on_closeButton_clicked() {
    this->onAccepted();
}

/**
 * @brief Gui_Rankings::on_actionExport_triggered exports event stats to a CSV file
 */
void Gui_Rankings::on_actionExport_triggered() {
    QString path;
    path = QFileDialog::getSaveFileName( this, this->tr( "Export statistics to CSV format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" ));

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
        foreach ( Team *teamPtr, m.currentEvent()->teamList ) {
            int points;

            if ( teamPtr->disqualified())
                points = -1;

            points = teamPtr->points() - teamPtr->penalty();

            if ( points <= 0 )
                points = 0;

            out << QString( "%1;%2;%3;%4;%5;%6%7" )
                   .arg( teamPtr->name())
                   .arg( teamPtr->logList.count())
                   .arg( teamPtr->combos())
                   .arg( teamPtr->timeOnTrack())
                   .arg( teamPtr->penalty())
                   .arg( points )
       #ifdef Q_OS_WIN
                   .arg( "\r\n" );
#else
                   .arg( "\n" );
#endif
        }
    }
    csv.close();
}

/**
 * @brief Gui_Rankings::on_actionRefresh_triggered recalculates statistics and repopulates rankingView
 */
void Gui_Rankings::on_actionRefresh_triggered() {
    this->calculateStatistics();
    this->clearData();
    this->fillData();
}

/**
 * @brief listToAscending
 * @param ePtr0
 * @param ePtr1
 * @return
 */
static bool listToAscending( const QString s0, const QString s1 ) {
    return m.transliterate( s0.toLower()) < m.transliterate( s1.toLower());
}

/**
 * @brief Gui_Rankings::on_actionNoLogs_triggered opens a dialog that shows tasks that have not been logged by any team
 */
void Gui_Rankings::on_actionNoLogs_triggered() {
#ifdef APPLET_DEBUG
    QStringList taskList;

    Event *event = m.currentEvent();
    if ( event == NULL )
        return;

    // build event taskList
    foreach ( Task *task, m.base.taskList ) {
        if ( task->eventId() == event->id())
            taskList << task->name();
    }

    // remove tasks if logged by any team
    foreach ( Log *log, m.base.logList ) {
        Task *task = m.taskForId( log->taskId());
        if ( task == NULL )
            continue;

        if ( task->eventId() != event->id())
            continue;

        if ( log->value() <= 0 )
            continue;

        taskList.removeOne( task->name());
    }

    qSort( taskList.begin(), taskList.end(), listToAscending );

    QDialog dialog;
    QHBoxLayout *layout = new QHBoxLayout();
    QListView *taskListView = new QListView();
    QStringListModel *stringListModel = new QStringListModel( taskList );
    taskListView->setModel( stringListModel );
    taskListView->setAlternatingRowColors( true );
    layout->addWidget( taskListView );
    layout->setMargin( 0 );
    dialog.setWindowTitle( this->tr( "Unlogged tasks" ));
    dialog.setLayout( layout );
    dialog.exec();
#endif
}
