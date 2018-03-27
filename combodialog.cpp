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
 * @brief ComboDialog::ComboDialog
 * @param parent
 */
ComboDialog::ComboDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::ComboDialog ) {
    int y;

    this->ui->setupUi( this );

    // connect for updates
    this->ui->comboTeams->setModel( Main::instance()->teamModel );
    this->connect<void( QComboBox::* )( int )>( this->ui->comboTeams, &QComboBox::currentIndexChanged, [ this ]( int index ) { this->currentTeamIndexChanged( this->ui->comboTeams->itemData( index, Qt::UserRole ).toInt()); } );

    // set up view
    this->comboModel = new ComboModel( this );
    this->ui->listCombined->setModel( this->comboModel );

    // trigger change
    this->ui->comboTeams->setCurrentIndex( -1 );
    for ( y = 0; y < this->ui->comboTeams->count(); y++ ) {
        if ( this->ui->comboTeams->itemData( y, Qt::UserRole ).toInt() == qobject_cast<MainWindow*>( parent )->currentTeamId())
            this->ui->comboTeams->setCurrentIndex( y );
    }

    // onClose lambda
    this->connect( this->ui->buttonClose, &QPushButton::clicked, [ this ]() { this->accept(); } );
}

/**
 * @brief ComboDialog::~ComboDialog
 */
ComboDialog::~ComboDialog() {
    // clean up
    this->logListSorted.clear();
    this->disconnect( this->ui->comboTeams, SIGNAL( currentIndexChanged( int )));
    delete this->comboModel;
    delete this->ui;
}

/**
 * @brief ComboDialog::currentTeamIndexChanged
 * @param index
 */
void ComboDialog::currentTeamIndexChanged( int index ) {
    Team *team;

    // abort on invalid indexes
    if ( index == -1 )
        return;

    // get current team
    team = Team::forId( index );
    if ( team == nullptr )
        return;

    // begin reset
    this->comboModel->beginReset();

    // make a local copy and sort it by comboId
    this->logListSorted = team->logList;
    qSort( this->logListSorted.begin(), this->logListSorted.end(), []( Log *l0, Log *l1 ) { return l0->comboId() < l1->comboId(); } );

    foreach ( Log *log, this->logListSorted ) {
        if ( log->comboId() == -1 )
            this->logListSorted.removeOne( log );
    }

    team->calculateCombos();
    this->ui->combos->setText( this->tr( "%1 (%2 tasks)" ).arg( team->combos()).arg( team->total()));
    this->ui->points->setText( QString( "%1" ).arg( team->bonus()));

    // end reset
    this->comboModel->endReset();
}
