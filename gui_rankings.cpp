/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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
    this->proxyModel = new QSortFilterProxyModel( this );
    this->modelPtr = new Gui_RankingModel();
    this->proxyModel->setSourceModel( this->modelPtr );
    this->proxyModel->setDynamicSortFilter( true );
    this->ui->rankingView->setModel( proxyModel );
    this->proxyModel->sort( Gui_RankingModel::Points, Qt::DescendingOrder );
    this->ui->rankingView->resizeColumnsToContents();
    this->ui->rankingView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    // set stats
    this->calculateStatistics();

    // scale window to contents
    this->rescaleWindow();
}

/*
================
calculateStatistics
================
*/
void Gui_Rankings::calculateStatistics() {
    int numParcipiants = 0, numTasks = 0;

    this->ui->tTeams->setText( QString( "%1\n" ).arg( m.teamList.count()));
    foreach ( TeamEntry *teamPtr, m.teamList ) {
        numParcipiants += teamPtr->members();
        numTasks += teamPtr->logList.count();
    }
    this->ui->tPar->setText( QString( "%1\n" ).arg( numParcipiants ));
    this->ui->tTasks->setText( QString( "%1\n" ).arg( numTasks ));
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
