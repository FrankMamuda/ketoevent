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
#include "editordialog.h"
#include "eventtoolbar.h"
#include "main.h"
#include "tasktoolbar.h"
#include "teamtoolbar.h"
#include "ui_editordialog.h"
#include "variable.h"
#include <QDebug>

// singleton
EditorDialog *EditorDialog::i = nullptr;

/**
 * @brief EditorDialog::EditorDialog
 * @param parent
 */
EditorDialog::EditorDialog() : container(nullptr), dock(nullptr), toolBar(nullptr), ui(new Ui::EditorDialog) {
    // set up ui
    ui->setupUi(this);
    container = ui->tableView;
    container->verticalHeader()->hide();
    dock = ui->dockWidget;
    hideDock();

    // connect close button
    connect(ui->buttonClose, &QPushButton::clicked, this, [this]() { close(); });

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief EditorDialog::~EditorDialog
 */
EditorDialog::~EditorDialog() {
    disconnect(ui->buttonClose, SIGNAL(clicked()));
    delete ui;
}

/**
 * @brief EditorDialog::isDockVisible
 * @return
 */
bool EditorDialog::isDockVisible() const { return dock->isVisible(); }

/**
 * @brief EditorDialog::showDock
 */
void EditorDialog::showDock(QWidget *contents, const QString &title) {
    dock->setWidget(contents);
    dock->setWindowTitle(title);
    dock->show();
    ui->buttonClose->hide();
    container->setDisabled(true);

    if (toolBar != nullptr) toolBar->setDisabled(true);
}

/**
 * @brief EditorDialog::hideDock
 */
void EditorDialog::hideDock() {
    dock->hide();
    ui->buttonClose->show();
    container->setEnabled(true);

    if (toolBar != nullptr) toolBar->setEnabled(true);
}

/**
 * @brief EditorDialog::setToolBar
 * @param toolBar
 */
void EditorDialog::setToolBar(QToolBar *widget) {
    removeToolBar(toolBar);
    toolBar = widget;
    addToolBar(Qt::TopToolBarArea, toolBar);
}

/**
 * @brief EditorDialog::showEvent
 * @param event
 */
void EditorDialog::showEvent(QShowEvent *event) {
    ModalWindow::showEvent(event);

    if (!isMaximized()) {
        if (toolBar == EventToolBar::instance() && !Variable::value<QVariant>("geometry/events").isNull())
            restoreGeometry(Variable::compressedByteArray("geometry/events"));

        if (toolBar == TaskToolBar::instance() && !Variable::value<QVariant>("geometry/tasks").isNull())
            restoreGeometry(Variable::compressedByteArray("geometry/tasks"));

        if (toolBar == TeamToolBar::instance() && !Variable::value<QVariant>("geometry/teams").isNull())
            restoreGeometry(Variable::compressedByteArray("geometry/teams"));
    }
}

/**
 * @brief EditorDialog::closeEvent
 * @param event
 */
void EditorDialog::closeEvent(QCloseEvent *event) {
    if (!isMaximized()) {
        if (toolBar == EventToolBar::instance()) Variable::setCompressedByteArray("geometry/events", saveGeometry());
        if (toolBar == TaskToolBar::instance()) Variable::setCompressedByteArray("geometry/tasks", saveGeometry());
        if (toolBar == TeamToolBar::instance()) Variable::setCompressedByteArray("geometry/teams", saveGeometry());
    }

    hideDock();
    disconnect(container, SIGNAL(clicked(QModelIndex)));
    ModalWindow::closeEvent(event);
}
