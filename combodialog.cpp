/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "combodialog.h"
#include "ui_combodialog.h"
#include "mainwindow.h"

/**
 * @brief listByCombos
 * @param lPtr0
 * @param lPtr1
 * @return
 */
static bool listByCombos( Log *lPtr0, Log *lPtr1 ) {
    return lPtr0->comboId() < lPtr1->comboId();
}

/**
 * @brief ComboDialog::ComboDialog
 * @param parent
 */
ComboDialog::ComboDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::ComboDialog ) {
    this->ui->setupUi( this );

    // connect for updates
    this->connect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )), this, SLOT( currentTeamIndexChanged( int )));

    // set up view
    this->comboModelPtr = new ComboModel( this );
    this->ui->listCombined->setModel( this->comboModelPtr );

    // fill combobox with team names & trigger change
    this->fillTeams();
}

/**
 * @brief ComboDialog::~ComboDialog
 */
ComboDialog::~ComboDialog() {
    // clean up
    this->logListSorted.clear();
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    delete this->comboModelPtr;
    delete ui;
}

/**
 * @brief ComboDialog::currentTeamIndexChanged
 * @param index
 */
void ComboDialog::currentTeamIndexChanged( int index ) {
    Team *teamPtr;

    // abort on invalid indexes
    if ( index == -1 )
        return;

    // get current team
    this->setCurrentTeamIndex( this->ui->comboTeams->itemData( index ).toInt());
    teamPtr = Team::forId( this->currentTeamIndex());
    if ( teamPtr == nullptr )
        return;

    // begin reset
    this->comboModelPtr->beginReset();

    // make a local copy and sort it by comboId
    this->logListSorted = teamPtr->logList;
    qSort( this->logListSorted.begin(), this->logListSorted.end(), listByCombos );

    foreach ( Log *logPtr, this->logListSorted ) {
        if ( logPtr->comboId() == -1 )
            this->logListSorted.removeOne( logPtr );
    }

    teamPtr->calculateCombos();
    this->ui->combos->setText( this->tr( "%1 (%2 tasks)" ).arg( teamPtr->combos()).arg( teamPtr->total()));
    this->ui->points->setText( QString( "%1" ).arg( teamPtr->bonus()));

    // end reset
    this->comboModelPtr->endReset();
}

/**
 * @brief ComboDialog::fillTeams
 */
void ComboDialog::fillTeams() {
    // clear list
    this->ui->comboTeams->clear();

    // repopulate list
    foreach ( Team *teamPtr, Event::active()->teamList )
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());

    // set to current team
    MainWindow *mPtr = qobject_cast<MainWindow *>( this->parent());
    if ( mPtr == nullptr )
        return;

    this->ui->comboTeams->setCurrentIndex( mPtr->currentTeamIndex());
}

/**
 * @brief ComboDialog::on_buttonClose_clicked
 */
void ComboDialog::on_buttonClose_clicked() {
    this->accept();
}
