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
#include "combomodel.h"
#include "combos.h"
#include "main.h"
#include "team.h"
#include "ui_combos.h"
#include "variable.h"

/**
 * @brief Combos::Combos
 */
Combos::Combos() : ui( new Ui::Combos ) {
    this->setWindowModality( Qt::ApplicationModal );
    this->ui->setupUi( this );
    this->ui->closeButton->setIcon( QIcon( ":/icons/close" ));
    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );

    // set window icon
    this->setWindowIcon( QIcon( ":/icons/combos" ));

    // connect for updates
    this->ui->view->setModel( ComboModel::instance());
    this->ui->teamCombo->setModel( Team::instance());
    this->ui->teamCombo->setModelColumn( Team::Title );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief Combos::~Combos
 */
Combos::~Combos() {
    this->disconnect( this->ui->closeButton, SIGNAL( clicked()));
    delete this->ui;
}

/**
 * @brief Combos::on_teamCombo_currentIndexChanged
 * @param index
 */
void Combos::on_teamCombo_currentIndexChanged( int index ) {
    ComboModel::instance()->reset( Team::instance()->id( index ));
    this->ui->view->reset();
    this->ui->combosEdit->setText( QString::number( ComboModel::instance()->combos ));
    this->ui->pointsEdit->setText( QString::number( ComboModel::instance()->points ));
}

/**
 * @brief Combos::showEvent
 * @param event
 */
void Combos::showEvent( QShowEvent *event ) {
    ModalWindow::showEvent( event );

    // set current team
    const int currentTeamRow = Team::instance()->row( MainWindow::instance()->currentTeamId());
    this->ui->teamCombo->setCurrentIndex( currentTeamRow );
}