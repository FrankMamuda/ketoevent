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
#include "about.h"
#include "ui_about.h"
#include <QMessageBox>

/**
 * @brief About::About
 * @param parent
 */
About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About) {
    ui->setupUi(this);
    QPushButton::connect(ui->closeButton, &QPushButton::clicked, this, [this]() { close(); });
    QPushButton::connect(ui->qtButton, &QPushButton::clicked, this, [this]() { QMessageBox::aboutQt(this); });
}

/**
 * @brief About::~About
 */
About::~About() {
    disconnect(ui->closeButton, SIGNAL(clicked()));
    disconnect(ui->qtButton, SIGNAL(clicked()));
    delete ui;
}
