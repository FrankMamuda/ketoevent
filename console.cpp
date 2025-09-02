/*
 * Copyright (C) 2013-2019 Factory #12
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
#include "console.h"
#include "cmd.h"
#include "ui_console.h"
#include "variable.h"

// singleton
Console *Console::i = nullptr;

/**
 * @brief Console::Console
 */
Console::Console() : ui(new Ui::Console) {
    ui->setupUi(this);
    edit = ui->input;
    setWindowFlags(Qt::Tool);
    setWindowOpacity(0.95);
    ui->screen->clear();

    // install event filter
    edit->installEventFilter(this);
    edit->history = Variable::string("system/consoleHistory").split(";");

    // announce
    print(tr("This is the console. Type 'help' if unsure what to do.\n"));
}

/**
 * @brief Console::~Console
 */
Console::~Console() {
    Variable::setValue("system/consoleHistory", edit->history.join(";"));
    edit->removeEventFilter(this);
    delete ui;
}

/**
 * @brief Console::completeCommand
 * @return
 */
bool Console::completeCommand() {
    int match = 0;
    QStringList matchedStrings;
    int y;

    // find matching commands
    const QStringList keys(Cmd::instance()->keys());
    for (const QString &name : keys) {
        if (name.startsWith(edit->text())) matchedStrings << name;
    }

    // find matching cvars
    for (const QSharedPointer<Var> &entry : std::as_const(Variable::instance()->list)) {
        if (!QString::compare(entry->key(), "system/consoleHistory")) continue;

        if (entry->key().startsWith(edit->text())) matchedStrings << entry->key();
    }

    // complete to shortest string
    if (matchedStrings.count() == 1) {
        // append extra space (since it's the only match that will likely be follwed by an argument)
        edit->setText(matchedStrings.first() + " ");
    } else if (matchedStrings.count() > 1) {
        match = 1;
        for (y = 0; y < matchedStrings.count(); y++) {
            // make sure we check string length
            if (matchedStrings.first().length() == match || matchedStrings.at(y).length() == match) break;

            if (matchedStrings.first().at(match) == matchedStrings.at(y).at(match)) {
                if (y == matchedStrings.count() - 1) {
                    match++;
                    y = 0;
                }
            }
        }
        edit->setText(matchedStrings.first().left(match));
    } else if (!matchedStrings.count()) {
        return true;
    }

    // print out suggestions
    qInfo() << tr("Available commands and cvars:");
    for (const QString &str : std::as_const(matchedStrings)) {
        // check commands
        if (Cmd::instance()->keys().contains(str)) {
            QString description(Cmd::instance()->description(str));
            qInfo() << (!description.isEmpty() ? QString("  \"%1\" - %2").arg(str, description) : QString("  \"%1").arg(str));
        }

        // check variables
        if (Variable::instance()->contains(str)) {
            QSharedPointer<Var> entry(Variable::instance()->list[str]);
            qInfo() << tr("  \"%1\" is \"%2\"").arg(entry->key(), entry->value().toString());
        }
    }

    // add extra newline
    qInfo() << "\n";
    return true;
}

/**
 * @brief ConsoleEventFilter::eventFilter
 * @param object
 * @param event
 * @return
 */
bool Console::eventFilter(QObject *object, QEvent *event) {
    HistoryEdit *edit(qobject_cast<HistoryEdit *>(object));

    if (edit == nullptr) return false;

    if (edit->hasFocus()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent(static_cast<QKeyEvent *>(event));

            if (keyEvent->key() == Qt::Key_Up) {
                if (!edit->history.isEmpty()) {
                    if (edit->offset() < edit->history.count()) edit->push();

                    const int offset = static_cast<int>(edit->history.count() - edit->offset());
                    edit->setText(offset > 0 ? edit->history.at(offset) : edit->history.first());
                }
                return true;
            } else if (keyEvent->key() == Qt::Key_Down) {
                if (!edit->history.isEmpty()) {
                    if (edit->offset() > 0) edit->pop();

                    if (edit->offset() == 0) {
                        edit->clear();
                        return true;
                    }

                    const int offset = static_cast<int>(edit->history.count() - edit->offset());
                    edit->setText(offset < edit->history.count() ? edit->history.at(offset) : edit->history.last());
                }
                return true;
            } else if (keyEvent->key() == Qt::Key_Tab) {
                if (edit->text().isEmpty()) return true;

                return completeCommand();
            }
        }
    }
    return false;
}

/**
 * @brief Console::print
 * @param msg
 */
void Console::print(const QString &msg) {
    QString out(msg);

    if (out.startsWith('"')) out = out.mid(1, out.length() - 2);

    ui->screen->append(out.replace("\\\"", "\""));

    // move cursor
    QTextCursor cursor(ui->screen->textCursor());
    cursor.movePosition(QTextCursor::End);
    ui->screen->setTextCursor(cursor);
}

/**
 * @brief Console::on_input_returnPressed
 */
void Console::on_input_returnPressed() {
    if (Cmd::instance()->execute(edit->text())) edit->add(edit->text());

    // set min offset
    edit->reset();
    edit->clear();
}
