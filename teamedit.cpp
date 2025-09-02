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
#include "teamedit.h"
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "team.h"
#include "ui_teamedit.h"
#include "variable.h"
#include <QMessageBox>

// singleton
TeamEdit *TeamEdit::i = nullptr;

/**
 * @brief TeamEdit::TeamEdit
 * @param parent
 */
TeamEdit::TeamEdit(QWidget *parent) : QWidget(parent), ui(new Ui::TeamEdit), m_edit(false) {
    const Row event = MainWindow::instance()->currentEvent();

    // set up ui
    ui->setupUi(this);

    // setup pixmaps
    ui->membersPixmap->setPixmap(QIcon::fromTheme("teams").pixmap(16, 16));
    ui->titlePixmap->setPixmap(QIcon::fromTheme("name").pixmap(16, 16));
    ui->reviewerPixmap->setPixmap(QIcon::fromTheme("ketone").pixmap(16, 16));

    if (event == Row::Invalid) return;

    // set up defaults
    ui->membersInteger->setMinimum(Event::instance()->minMembers(event));
    ui->membersInteger->setMaximum(Event::instance()->maxMembers(event));
    ui->finishTime->setMinimumTime(Event::instance()->startTime(event));
    ui->finishTime->setMaximumTime(Event::instance()->finalTime(event).addSecs(10800));

    // only visible in quick add
    setWindowTitle(tr("Add team"));

    // empty team title check
    auto emptyTitle = [this]() {
        // warn upon empty team title
        if (ui->titleEdit->text().isEmpty()) {
            QMessageBox::information(this, tr("Empty team title"), tr("Please enter team title"));
            return true;
        }
        return false;
    };

    // add button action
    connect(ui->addButton, &QPushButton::clicked, this, [this, emptyTitle]() {
        const QString teamTitle(ui->titleEdit->text());

        // abort on empty team title
        if (emptyTitle()) return;

        // abort on existing team
        if (Team::instance()->contains(Team::Title, teamTitle) && !isEditing()) {
            QMessageBox::information(this, tr("Team already exists"), tr("Team already exists\nChoose a different title"));
            return;
        }

        // if everything is ok, add a new team
        Row team = Row::Invalid;
        if (!isEditing()) {
            team = Team::instance()->add(teamTitle, ui->membersInteger->value(), ui->finishTime->time(), ui->reviewerEdit->text());

        } else {
            const Row team = Team::instance()->row(EditorDialog::instance()->container->currentIndex().row());

            if (team == Row::Invalid) return;

            Team::instance()->setTitle(team, teamTitle);
            Team::instance()->setMembers(team, ui->membersInteger->value());
            Team::instance()->setFinishTime(team, ui->finishTime->time());
            Team::instance()->setReviewer(team, ui->reviewerEdit->text());
        }

        if (team != Row::Invalid) MainWindow::instance()->setCurrentTeam(team);

        // close dock
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
        else close();
    });

    // shortcut from title to members
    connect(ui->titleEdit, &QLineEdit::returnPressed, this, [this, emptyTitle]() {
        if (emptyTitle()) return;

        ui->membersInteger->setFocus();
    });

    // shortcut from members to time
    connect(ui->membersInteger, &QSpinBox::editingFinished, this, [this]() { ui->finishTime->setFocus(); });

    // shortcut from time to add button
    connect(ui->finishTime, &QTimeEdit::editingFinished, this, [this]() {
        ui->addButton->setFocus();
        ui->addButton->setDefault(true);
        ui->addButton->setAutoDefault(true);
    });

    // cancel button just closes the dialog
    connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
        else close();
    });

    // connect time button
    connect(ui->finishButton, &QToolButton::pressed, this, [this]() { setCurrentTime(); });

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief TeamEdit::~TeamEdit
 */
TeamEdit::~TeamEdit() {
    // disconnect lambdas
    disconnect(ui->addButton, SIGNAL(clicked()));
    disconnect(ui->cancelButton, SIGNAL(clicked()));
    disconnect(ui->titleEdit, SIGNAL(returnPressed()));
    disconnect(ui->membersInteger, SIGNAL(editingFinished()));
    disconnect(ui->finishTime, SIGNAL(editingFinished()));
    disconnect(ui->finishButton, SIGNAL(pressed()));

    // delete ui
    delete ui;
}

/**
 * @brief TeamEdit::reset
 */
void TeamEdit::reset(bool edit) {
    m_edit = edit;

    if (!isEditing()) {
        // reset ui components to default values
        ui->titleEdit->clear();
        ui->finishTime->setTime(ui->finishTime->minimumTime());
        ui->membersInteger->setValue(EventTable::DefaultMembers);
        ui->reviewerEdit->setText(Variable::string("reviewerName"));
    } else {
        const Row team = Team::instance()->row(EditorDialog::instance()->container->currentIndex().row());

        if (team == Row::Invalid) return;

        ui->titleEdit->setText(Team::instance()->title(team));
        ui->finishTime->setTime(Team::instance()->finishTime(team));
        ui->membersInteger->setValue(Team::instance()->members(team));
        ui->reviewerEdit->setText(Team::instance()->reviewer(team));
    }

    ui->titleEdit->setFocus();
    ui->addButton->setDefault(false);
    ui->addButton->setAutoDefault(false);
}

/**
 * @brief TeamEdit::setCurrentTime
 */
void TeamEdit::setCurrentTime() { ui->finishTime->setTime(QTime::currentTime()); }
