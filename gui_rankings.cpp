/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
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

/*
================
construct
================
*/
Gui_Rankings::Gui_Rankings( QWidget *parent ) : Gui_SettingsDialog( parent ), ui( new Ui::Gui_Rankings ) {
    // set up ui
    ui->setupUi( this );

    // failsafe
    if ( m.isInitialized())
        this->bindVars();
    else
        this->reject();

    // set up ranking view
    this->ui->rankingView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    this->ui->rankingView->setSortingEnabled( true );

    // set up sorting & view model
    this->proxyModel = new RankingsSortModel( this );
    this->modelPtr = new QStandardItemModel( m.currentEvent()->teamList.count(), Rankings::NumRankingColumns, this );
    this->proxyModel->setSourceModel( this->modelPtr );
    this->proxyModel->setDynamicSortFilter( true );
    this->ui->rankingView->verticalHeader()->hide();
    this->ui->rankingView->setModel( this->proxyModel );

    // connect
    this->connect( m.cvar( "rankings/current" ), SIGNAL( changed()), this, SLOT( fillData()));

    // set stats
    this->calculateStatistics();
    this->fillData();
    this->proxyModel->sort( Rankings::Points, Qt::DescendingOrder );

    // scale window to contents
    this->ui->rankingView->resizeColumnsToContents();
    this->rescaleWindow();

    // disable horizontal scrollbar
    this->ui->rankingView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
}

/*
================
lessThan
================
*/
static bool greaterThan( int a, int b ) {
    return a > b;
}

/*
================
fillData
================
*/
void Gui_Rankings::fillData() {
    unsigned int y;
    int k, points;
    QFont boldFont;
    boldFont.setBold( true );
    QList<int> pointsList;

    // generate header
    for ( y = 0; y < Rankings::NumRankingColumns; y++ ) {
        QStandardItem *itemPtr = new QStandardItem( Rankings::ColumnHeaders[y] );
        itemPtr->setFont( boldFont );
        itemPtr->setData( Qt::AlignCenter, Qt::TextAlignmentRole );
        this->modelPtr->setHorizontalHeaderItem( y, itemPtr );
    }

    // fill data
    for ( y = 0; y < Rankings::NumRankingColumns; y++ ) {
        for ( k = 0; k < m.currentEvent()->teamList.count(); k++ ) {
            TeamEntry *teamPtr = m.currentEvent()->teamList.at( k );
            Gui_Main *guiPtr = qobject_cast<Gui_Main*>( this->parent());
            QStandardItem *itemPtr = new QStandardItem();
            QString text;

            if ( teamPtr == NULL || guiPtr == NULL )
                return;

            switch ( y ) {
            case Rankings::TeamName:
                text = teamPtr->name();

                if ( m.cvar( "rankings/current" )->isEnabled()) {
                    if ( teamPtr != m.teamForId( guiPtr->currentTeamId()))
                        text = "";
                }
                break;

            case Rankings::Tasks:
                text = QString( "%1" ).arg( teamPtr->logList.count());
                break;

            case Rankings::Combos:
                text = QString( "%1" ).arg( teamPtr->combos());
                break;

            case Rankings::Total:
                text = QString( "%1" ).arg( teamPtr->total());
                break;

            case Rankings::Time:
                text = QString( "%1" ).arg( teamPtr->timeOnTrack());
                break;

            case Rankings::Reviewer:
                text = teamPtr->reviewer();
                break;

            case Rankings::Penalty:
                if ( teamPtr->penalty() > 0 )
                    itemPtr->setForeground( QColor( Qt::red ));
                text = QString( "%1" ).arg( teamPtr->penalty());
                break;

            case Rankings::Points:
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

            // bold points
            //if ( y == Rankings::Points )
            //    itemPtr->setFont( boldFont );

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
        int rankColumn = Rankings::Rank;
        int pointsColumn = Rankings::Points;
        this->modelPtr->item( k, rankColumn )->setText( QString( "%1" ).arg( pointsList.indexOf( this->modelPtr->item( k, pointsColumn )->text().toInt()) + 1 ));
    }
}

/*
================
calculateStatistics
================
*/
void Gui_Rankings::calculateStatistics() {
    int numParcipiants = 0, numTasks = 0, pointsLogged = 0, maxPoints = 0;

    // get team stats
    foreach ( TeamEntry *teamPtr, m.currentEvent()->teamList ) {
        numParcipiants += teamPtr->members();
        numTasks += teamPtr->logList.count();
        teamPtr->calculateCombos();
        pointsLogged += teamPtr->points();
    }

    // get max points
    foreach ( TaskEntry *taskPtr, m.currentEvent()->taskList ) {
        if ( taskPtr->type() == TaskEntry::Check )
            maxPoints += taskPtr->points();
        else if ( taskPtr->type() == TaskEntry::Multi )
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

/*
================
rescaleWindow
================
*/
void Gui_Rankings::rescaleWindow() {
      QRect tableRect, windowRect, parentRect;
      int y, columnWidths = 0;

      // scale window to contents
      tableRect = this->ui->rankingView->geometry();
      windowRect = this->geometry();
      parentRect = this->parentWidget()->geometry();

      for ( y = 0; y < static_cast<int>( Rankings::NumRankingColumns ); y++ )
          columnWidths += this->ui->rankingView->columnWidth( y );

      // this looks fairly fine
      tableRect.setWidth( 2 + this->ui->rankingView->verticalHeader()->width() + columnWidths );
      this->ui->rankingView->setGeometry( tableRect );
      windowRect = this->geometry();
      windowRect.setWidth( 24 + tableRect.width() + this->ui->layoutTeamRankings->margin() * 2 + this->ui->groupRankings->layout()->margin() * 2 );
      this->setFixedWidth( windowRect.width());
}

/*
================
destruct
================
*/
Gui_Rankings::~Gui_Rankings() {
    delete ui;
    this->unbindVars();

    // this should trigger deletion of QStandardItems
    this->modelPtr->clear();

    // disconnect
    this->disconnect( m.cvar( "rankings/current" ), SIGNAL( changed()), this, SLOT( fillData()));

    // delete sort and data models
    delete this->modelPtr;
    delete this->proxyModel;
}

/*
================
bindVars
================
*/
void Gui_Rankings::bindVars() {
    // lock vars
    this->lockVariables();

    // bind vars
    this->bindVariable( "rankings/current", this->ui->checkCurrent );

    // unlock vars
    this->lockVariables( false );
}

/*
================
exportButton
================
*/
void Gui_Rankings::on_exportButton_clicked() {
    QString path;
    path = QFileDialog::getSaveFileName( this, this->tr( "Export statistics to CSV format" ), QDir::homePath(), this->tr( "CSV file (*.csv)" ));

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
        foreach ( TeamEntry *teamPtr, m.currentEvent()->teamList ) {
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
