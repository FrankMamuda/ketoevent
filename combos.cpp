/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
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
    this->connect( this->ui->closeButton, &QPushButton::clicked, [ this ]() { this->close(); } );

    // set up view
    this->ui->view->setModel( &ComboModel::instance());
    this->ui->teamCombo->setModel( &Team::instance());
    this->ui->teamCombo->setModelColumn( Team::Title );

    // set up pixmaps
    this->ui->teamPixmap->setPixmap( QIcon::fromTheme( "teams" ).pixmap( 16, 16 ));
    this->ui->comboPixmap->setPixmap( QIcon::fromTheme( "combos" ).pixmap( 16, 16 ));
    this->ui->pointsPixmap->setPixmap( QIcon::fromTheme( "star" ).pixmap( 16, 16 ));
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
    const Row row = Team::instance().row( index );

    ComboModel::instance().reset( row == Row::Invalid ? Id::Invalid : Team::instance().id( row ));

    this->ui->view->reset();
    this->ui->combosEdit->setText( QString::number( ComboModel::instance().combos ));
    this->ui->pointsEdit->setText( QString::number( ComboModel::instance().points ));}

/**
 * @brief Combos::showEvent
 * @param event
 */
void Combos::showEvent( QShowEvent *event ) {
    ModalWindow::showEvent( event );

    // set current team
    const Row row = MainWindow::instance().currentTeam();

    // reset model on every show just to be safe
    this->ui->teamCombo->setCurrentIndex( static_cast<int>( row ));
    this->on_teamCombo_currentIndexChanged( static_cast<int>( row ));
}
