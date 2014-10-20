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

/*
================
construct
================
*/
Gui_Rankings::Gui_Rankings( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Rankings ) {
    // set up ui
    ui->setupUi( this );

    // set up ranking view
    this->ui->rankingView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    this->ui->rankingView->setSortingEnabled( true );

    // set up sorting & view model
    // fixme: bottleneck (proxy sort)
    this->proxyModel = new QSortFilterProxyModel( this );
    this->modelPtr = new Gui_RankingModel();
    this->proxyModel->setSourceModel( this->modelPtr );
    this->proxyModel->setDynamicSortFilter( true );
    this->ui->rankingView->setModel( proxyModel );
    this->ui->rankingView->resizeColumnsToContents();

    // set stats
    this->calculateStatistics();
    this->proxyModel->sort( Gui_RankingModel::Points, Qt::DescendingOrder );

    // scale window to contents
    this->rescaleWindow();

    // disable horizontal scrollbar
    this->ui->rankingView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
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
    delete this->modelPtr;
    delete this->proxyModel;
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
