/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020-2024 Armands Aleksejevs
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
#include "combos.h"
#include "combomodel.h"
#include "main.h"
#include "team.h"
#include "ui_combos.h"

// singleton
Combos *Combos::i = nullptr;

/**
 * @brief Combos::Combos
 */
Combos::Combos() : ui(new Ui::Combos) {
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    connect(ui->closeButton, &QPushButton::clicked, [this]() { close(); });

    // set up view
    ui->view->setModel(ComboModel::instance());
    ui->teamCombo->setModel(Team::instance());
    ui->teamCombo->setModelColumn(Team::Title);

    // set up pixmaps
    ui->teamPixmap->setPixmap(QIcon::fromTheme("teams").pixmap(16, 16));
    ui->comboPixmap->setPixmap(QIcon::fromTheme("combos").pixmap(16, 16));
    ui->pointsPixmap->setPixmap(QIcon::fromTheme("star").pixmap(16, 16));

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief Combos::~Combos
 */
Combos::~Combos() {
    disconnect(ui->closeButton, SIGNAL(clicked()));

    delete ui;
}

/**
 * @brief Combos::on_teamCombo_currentIndexChanged
 * @param index
 */
void Combos::on_teamCombo_currentIndexChanged(int index) {
    const Row row = Team::instance()->row(index);

    ComboModel::instance()->reset(row == Row::Invalid ? Id::Invalid : Team::instance()->id(row));

    ui->view->reset();
    ui->combosEdit->setText(QString::number(ComboModel::instance()->combos));
    ui->pointsEdit->setText(QString::number(ComboModel::instance()->points));
}

/**
 * @brief Combos::showEvent
 * @param event
 */
void Combos::showEvent(QShowEvent *event) {
    ModalWindow::showEvent(event);

    // set current team
    const Row row = MainWindow::instance()->currentTeam();

    // reset model on every show just to be safe
    ui->teamCombo->setCurrentIndex(static_cast<int>(row));
    on_teamCombo_currentIndexChanged(static_cast<int>(row));
}
