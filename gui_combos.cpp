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
#include "gui_combos.h"
#include "ui_gui_combos.h"
#include "gui_main.h"

/*
================
listByCombos
================
*/
static bool listByCombos( LogEntry *lPtr0, LogEntry *lPtr1 ) {
    return lPtr0->comboId() < lPtr1->comboId();
}

/*
================
construct
================
*/
Gui_Combos::Gui_Combos( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Combos ) {
    ui->setupUi( this );

    // connect for updates
    this->connect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )), this, SLOT( currentTeamIndexChanged( int )));

    // set up view
    this->comboModelPtr = new Gui_ComboModel( this );
    this->ui->listCombined->setModel( this->comboModelPtr );

    // fill combobox with team names & trigger change
    this->fillTeams();
}

/*
================
destruct
================
*/
Gui_Combos::~Gui_Combos() {
    // clean up
    this->logListSorted.clear();
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    delete this->comboModelPtr;
    delete ui;
}

/*
================
currentTeamIndexChanged
================
*/
void Gui_Combos::currentTeamIndexChanged( int index ) {
    TeamEntry *teamPtr;

    // abort on invalid indexes
    if ( index == -1 )
        return;

    // get current team
    this->setCurrentTeamIndex( this->ui->comboTeams->itemData( index ).toInt());
    teamPtr = m.teamForId( this->currentTeamIndex());
    if ( teamPtr == NULL )
        return;

    // begin reset
    this->comboModelPtr->beginReset();

    // make a local copy and sort it by comboId
    this->logListSorted = teamPtr->logList;
    qSort( this->logListSorted.begin(), this->logListSorted.end(), listByCombos );

    //QList<QPair<int, int> >combos;
    foreach ( LogEntry *logPtr, this->logListSorted ) {
        if ( logPtr->comboId() == -1 )
            this->logListSorted.removeOne( logPtr );
    }

    //int points, combos, total;
    Main::stats_t stats = m.getComboStats( teamPtr->id());
    this->ui->combos->setText( this->tr( "%1 (%2 tasks)" ).arg( stats.combos ).arg( stats.total ));
    this->ui->points->setText( QString( "%1" ).arg( stats.points ));

    // end reset
    this->comboModelPtr->endReset();
}

/*
================
fillTeams
================
*/
void Gui_Combos::fillTeams() {
    // clear list
    this->ui->comboTeams->clear();

    // repopulate list
    foreach ( TeamEntry *teamPtr, m.teamList )
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());

    // set to current team
    GetPtr( Gui_Main *, mPtr, this->parent()); TestPtr( mPtr ) return;
    this->ui->comboTeams->setCurrentIndex( mPtr->currentTeamIndex());
}

/*
================
buttonClose->clicked
================
*/
void Gui_Combos::on_buttonClose_clicked() {
    this->accept();
}
