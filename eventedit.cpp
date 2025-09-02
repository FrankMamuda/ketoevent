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
#include "eventedit.h"
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "ui_eventedit.h"
#include <QMessageBox>

// singleton
EventEdit *EventEdit::i = nullptr;

/**
 * @brief EventEdit::EventEdit
 * @param parent
 */
EventEdit::EventEdit(QWidget *parent) : QWidget(parent), ui(new Ui::EventEdit), m_edit(false) {
    // set up ui
    ui->setupUi(this);

    // setup pixmaps
    ui->titlePixmap->setPixmap(QIcon::fromTheme("name").pixmap(16, 16));

    // only visible in quick add
    setWindowTitle(tr("Add event"));

    // empty event title check
    auto emptyTitle = [this]() {
        // warn upon empty event title
        if (ui->titleEdit->text().isEmpty()) {
            QMessageBox::information(this, tr("Empty event title"), tr("Please enter event title"));
            return true;
        }
        return false;
    };

    // add button action
    connect(ui->addButton, &QPushButton::clicked, this, [this, emptyTitle]() {
        const QString eventTitle(ui->titleEdit->text());

        // abort on empty event title
        if (emptyTitle()) return;

        // abort on existing event
        if (Event::instance()->contains(Event::Title, eventTitle) && !isEditing()) {
            QMessageBox::information(this, tr("Event already exists"), tr("Event already exists\nChoose a different title"));
            return;
        }

        // if everything is ok, add a new event
        Row event = Row::Invalid;
        if (!isEditing()) {
            event = Event::instance()->add(eventTitle, ui->minInteger->value(), ui->maxInteger->value(), ui->startTime->time(),
                ui->finishTime->time(), ui->finalTime->time(), ui->penaltyInteger->value(), ui->twoInteger->value(),
                ui->threeInteger->value(), ui->fourPlusInteger->value());
        } else {
            const Row event = Event::instance()->row(EditorDialog::instance()->container->currentIndex().row());

            if (event == Row::Invalid) return;

            Event::instance()->setTitle(event, eventTitle);
            Event::instance()->setMinMembers(event, ui->minInteger->value());
            Event::instance()->setMaxMembers(event, ui->maxInteger->value());
            Event::instance()->setStartTime(event, ui->startTime->time());
            Event::instance()->setFinishTime(event, ui->finishTime->time());
            Event::instance()->setFinalTime(event, ui->finalTime->time());
            Event::instance()->setPenaltyPoints(event, ui->penaltyInteger->value());
            Event::instance()->setComboOfTwo(event, ui->twoInteger->value());
            Event::instance()->setComboOfThree(event, ui->threeInteger->value());
            Event::instance()->setComboOfFourPlus(event, ui->fourPlusInteger->value());
        }

        if (event != Row::Invalid) MainWindow::instance()->setCurrentEvent(event);

        // close dock
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
        else close();
    });

    // cancel button just closes the dialog
    connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {
        if (EditorDialog::instance()->isDockVisible()) EditorDialog::instance()->hideDock();
        else close();
    });

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief EventEdit::~EventEdit
 */
EventEdit::~EventEdit() {
    // disconnect lambdas
    disconnect(ui->addButton, SIGNAL(clicked()));
    disconnect(ui->cancelButton, SIGNAL(clicked()));
    disconnect(ui->titleEdit, SIGNAL(returnPressed()));

    // delete ui
    delete ui;
}

/**
 * @brief EventEdit::reset
 * @param edit
 */
void EventEdit::reset(bool edit) {
    m_edit = edit;

    if (!isEditing()) {
        // reset ui components to default values
        ui->titleEdit->clear();
        ui->minInteger->setValue(EventTable::DefaultMinMembers);
        ui->maxInteger->setValue(EventTable::DefaultMaxMembers);
        ui->startTime->setTime(QTime::fromString(EventTable::DefaultStartTime, Database_::TimeFormat));
        ui->finishTime->setTime(QTime::fromString(EventTable::DefaultFinishTime, Database_::TimeFormat));
        ui->finalTime->setTime(QTime::fromString(EventTable::DefaultFinalTime, Database_::TimeFormat));
        ui->penaltyInteger->setValue(EventTable::DefaultPenaltyPoints);
        ui->twoInteger->setValue(EventTable::DefaultComboOfTwo);
        ui->threeInteger->setValue(EventTable::DefaultComboOfThree);
        ui->fourPlusInteger->setValue(EventTable::DefaultComboOfFourAndMore);
    } else {
        const Row event = Event::instance()->row(EditorDialog::instance()->container->currentIndex().row());

        if (event == Row::Invalid) return;

        ui->titleEdit->setText(Event::instance()->title(event));
        ui->minInteger->setValue(Event::instance()->minMembers(event));
        ui->maxInteger->setValue(Event::instance()->maxMembers(event));
        ui->startTime->setTime(Event::instance()->startTime(event));
        ui->finishTime->setTime(Event::instance()->finishTime(event));
        ui->finalTime->setTime(Event::instance()->finalTime(event));
        ui->penaltyInteger->setValue(Event::instance()->penalty(event));
        ui->twoInteger->setValue(Event::instance()->comboOfTwo(event));
        ui->threeInteger->setValue(Event::instance()->comboOfThree(event));
        ui->fourPlusInteger->setValue(Event::instance()->comboOfFourPlus(event));
    }

    ui->titleEdit->setFocus();
    ui->addButton->setDefault(false);
    ui->addButton->setAutoDefault(false);
}
