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
#include "settings.h"
#include "main.h"
#include "theme.h"
#include "ui_settings.h"
#include "variable.h"
#include <QFileDialog>
#include <QMessageBox>

// singleton
Settings *Settings::i = nullptr;

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings() : ui(new Ui::Settings) {
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);

    // setup pixmaps
    ui->reviewerPixmap->setPixmap(QIcon::fromTheme("ketone").pixmap(16, 16));

    const QStringList themes(Theme::availableThemes().keys());
    ui->themeCombo->clear();
    for (const QString &themeName : themes) ui->themeCombo->addItem(themeName, themeName);

    Settings::connect(ui->overrideCheck, &QCheckBox::toggled, this, [this](bool checked) { ui->themeCombo->setEnabled(checked); });

    // bind variables
    variables << Variable::instance()->bind("overrideTheme", ui->overrideCheck);
    variables << Variable::instance()->bind("theme", ui->themeCombo);
    variables << Variable::instance()->bind("reviewerName", ui->reviewerEdit);
    variables << Variable::instance()->bind("sortByType", ui->sortByTypeCheck);

    connect(ui->closeButton, &QPushButton::clicked, [this]() { close(); });

    // handle database path
    connect(ui->pathButton, &QPushButton::clicked, [this]() {
        const QString fileName(QFileDialog::getSaveFileName(this, tr("Open database"), QFileInfo(Variable::string("databasePath")).absolutePath(),
            tr("Database (*.db *.sqlite)"), nullptr, QFileDialog::DontConfirmOverwrite));

        if (fileName.isEmpty()) {
            QMessageBox::warning(this, tr("Settings"), tr("Invalid database selection"), QMessageBox::Close);
            return;
        }

        QMessageBox::warning(this, tr("Settings"), tr("Application will be restarted"), QMessageBox::Ok);
        Variable::setString("databasePath", fileName);
        QApplication::quit();
    });

    // bind database path to edit
    Variable::instance()->bind("databasePath", ui->pathEdit);
    Variable::instance()->bind("backup/enabled", ui->backupCheck);
    Variable::instance()->bind("backup/changes", ui->backupValue);

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    // unbind vars
    for (const QString &key : std::as_const(variables)) Variable::instance()->unbind(key);

    disconnect(ui->closeButton, SIGNAL(clicked()));
    delete ui;
}
