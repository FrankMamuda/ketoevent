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
#include "taskedit.h"
#include "editordialog.h"
#include "main.h"
#include "task.h"
#include "ui_taskedit.h"
#include <QCommonStyle>
#include <QDebug>
#include <QMessageBox>

// singleton
TaskEdit *TaskEdit::i = nullptr;

/**
 * @brief TaskEdit::TaskEdit
 * @param parent
 */
TaskEdit::TaskEdit(QWidget *parent) : QWidget(parent), ui(new Ui::TaskEdit), m_edit(false) {
    // set up defaults
    ui->setupUi(this);

    // setup pixmaps
    ui->namePixmap->setPixmap(QIcon::fromTheme("name").pixmap(16, 16));
    ui->multiPixmap->setPixmap(QIcon::fromTheme("multi").pixmap(16, 16));
    ui->stylePixmap->setPixmap(QIcon::fromTheme("style").pixmap(16, 16));
    ui->descPixmap->setPixmap(QIcon::fromTheme("info").pixmap(16, 16));
    ui->typePixmap->setPixmap(QIcon::fromTheme("type").pixmap(16, 16));
    ui->pointsPixmap->setPixmap(QIcon::fromTheme("star").pixmap(16, 16));

    // setup comboboxes
    connect<void (QComboBox::*)(int)>(ui->typeCombo, &QComboBox::activated, this, [this](int index) {
        const Task::Types type = static_cast<Task::Types>(index);

        if (type == Task::Types::Check) {
            ui->multiLabel->hide();
            ui->multiInteger->hide();
            ui->multiPixmap->hide();
            ui->patternEdit->hide();
            ui->patternLabel->hide();
        } else if (type == Task::Types::Multi) {
            ui->multiLabel->show();
            ui->multiInteger->show();
            ui->multiPixmap->show();
            ui->patternEdit->show();
            ui->patternLabel->show();
        }
    });
    ui->typeCombo->addItems(Task::instance()->types.values());
    ui->styleCombo->addItems(Task::instance()->styles.values());

    // empty task name check
    auto emptyName = [this]() {
        // warn upon empty task name
        if (ui->nameEdit->text().isEmpty()) {
            QMessageBox::information(this, tr("Empty task name"), tr("Please enter task name"));
            return true;
        }
        return false;
    };

    // add button action
    connect(ui->addButton, &QPushButton::clicked, this, [this, emptyName]() {
        const QString taskName(ui->nameEdit->text());

        // abort on empty task name
        if (emptyName()) return;

        // abort on existing task
        if (Task::instance()->contains(Task::Name, taskName) && !isEditing()) {
            QMessageBox::information(this, tr("Task already exists"), tr("Task already exists\nChoose a different name"));
            return;
        }

        // if everything is ok, add a new task
        if (!isEditing()) {
            Task::instance()->add(taskName, ui->pointsInteger->value(), ui->multiInteger->value(),
                static_cast<Task::Types>(ui->typeCombo->currentIndex()), static_cast<Task::Styles>(ui->styleCombo->currentIndex()),
                ui->descEdit->text(), ui->patternEdit->text());
        } else {
            const Row task = Task::instance()->row(EditorDialog::instance()->container->currentIndex().row());

            if (task == Row::Invalid) return;

            Task::instance()->setName(task, taskName);
            Task::instance()->setPoints(task, ui->pointsInteger->value());
            Task::instance()->setMulti(task, ui->multiInteger->value());
            Task::instance()->setType(task, static_cast<Task::Types>(ui->typeCombo->currentIndex()));
            Task::instance()->setStyle(task, static_cast<Task::Styles>(ui->styleCombo->currentIndex()));
            Task::instance()->setDescription(task, ui->descEdit->text());
            Task::instance()->setPattern(task, ui->patternEdit->text());
        }

        // close dock
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
    });

    // cancel button just closes the dialog
    connect(ui->cancelButton, &QPushButton::clicked, []() {
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
    });

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief TaskEdit::~TaskEdit
 */
TaskEdit::~TaskEdit() {
    // disconnect lambdas
    disconnect(ui->addButton, SIGNAL(clicked()));
    disconnect(ui->cancelButton, SIGNAL(clicked()));

    // delete ui
    delete ui;
}

/**
 * @brief TaskEdit::reset
 */
void TaskEdit::reset(bool edit) {
    m_edit = edit;

    if (!isEditing()) {
        ui->nameEdit->clear();
        ui->pointsInteger->setValue(1);
        ui->multiInteger->setValue(ui->multiInteger->minimum());
        ui->typeCombo->setCurrentIndex(0);
        ui->styleCombo->setCurrentIndex(0);
        ui->descEdit->clear();
        ui->patternEdit->clear();
    } else {
        const Row task = Task::instance()->row(EditorDialog::instance()->container->currentIndex().row());

        if (task == Row::Invalid) return;

        ui->nameEdit->setText(Task::instance()->name(task));
        ui->pointsInteger->setValue(Task::instance()->points(task));
        ui->multiInteger->setValue(Task::instance()->multi(task));
        ui->typeCombo->setCurrentIndex(static_cast<int>(Task::instance()->type(task)));
        ui->styleCombo->setCurrentIndex(static_cast<int>(Task::instance()->style(task)));
        ui->descEdit->setText(Task::instance()->description(task));
        ui->patternEdit->setText(Task::instance()->pattern(task));
    }

    ui->nameEdit->setFocus();
    ui->addButton->setDefault(false);
    ui->addButton->setAutoDefault(false);
}
