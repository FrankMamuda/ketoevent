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
#include "eventtoolbar.h"
#include "database.h"
#include "editordialog.h"
#include "event.h"
#include "eventedit.h"
#include "main.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

// singleton
EventToolBar *EventToolBar::i = nullptr;

/**
 * @brief EventToolBar::EventToolBar
 * @param parent
 */
EventToolBar::EventToolBar(QWidget *parent) : ToolBar(parent) {
    // add action
    addAction(QIcon::fromTheme("add"), tr("Add Event"), [this]() {
        if (!EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock(EventEdit::instance(), tr("Add Event"));
            EventEdit::instance()->reset();
        }
    });

    // edit action
    edit = addAction(QIcon::fromTheme("edit"), tr("Edit Event"), [this]() {
        if (!EditorDialog::instance()->isDockVisible()) {
            EditorDialog::instance()->showDock(EventEdit::instance(), tr("Edit Event"));
            EventEdit::instance()->reset(true);
        }
    });

    // remove action
    remove = addAction(QIcon::fromTheme("remove"), tr("Remove Event"), [this]() {
        const QModelIndex index(EditorDialog::instance()->container->currentIndex());

        if (EditorDialog::instance()->isDockVisible() || !index.isValid()) return;

        const Row row = Event::instance()->row(index);
        if (row == Row::Invalid) return;

        const QString title(Event::instance()->title(row));
        const Row event = MainWindow::instance()->currentEvent();

        if (QMessageBox::question(this, tr("Remove event"), tr("Do you really want to remove \"%1\"?").arg(title)) == QMessageBox::Yes) {
            Event::instance()->remove(row);
            Database::instance()->removeOrphanedEntries();
        }

        // restore eventId (model resets on remove apparently)
        if (event != row) MainWindow::instance()->setCurrentEvent(event);
    });

    // import action
    addAction(QIcon::fromTheme("export"), tr("Import logs"), [this]() {
        const QFileInfo info(
            QFileDialog::getOpenFileName(this, tr("Import logs from database"), QDir::homePath() + "/" + "database.db", tr("Database (*.db)")));
        if (!info.exists()) return;

        if (QMessageBox::question(this, tr("Import event"), tr("Do you really want to import logs and teams from \"%1\"?").arg(info.fileName()))
            == QMessageBox::Yes) {
            Database::instance()->attach(info);
        }
    });

    // button test (disconnected in ~EditorDialog)
    connect(EditorDialog::instance()->container, SIGNAL(clicked(QModelIndex)), this, SLOT(buttonTest(QModelIndex)));
    buttonTest();

    // add to garbage man
    GarbageMan::instance()->add(this);
}

/**
 * @brief EventToolBar::buttonTest
 * @param index
 */
void EventToolBar::buttonTest(const QModelIndex &index) {
    edit->setEnabled(index.isValid());
    remove->setEnabled(index.isValid());
};
