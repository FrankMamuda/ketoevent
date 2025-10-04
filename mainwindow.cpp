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
#include "mainwindow.h"
#include "about.h"
#include "combos.h"
#include "console.h"
#include "database.h"
#include "delegate.h"
#include "editordialog.h"
#include "event.h"
#include "eventtoolbar.h"
#include "log.h"
#include "main.h"
#include "rankings.h"
#include "settings.h"
#include "task.h"
#include "tasktoolbar.h"
#include "team.h"
#include "teamedit.h"
#include "teamtoolbar.h"
#include "ui_mainwindow.h"
#include "variable.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QSqlQuery>
#ifdef __APPLE__
#include <QMenu>
#endif

// singleton
MainWindow *MainWindow::i = nullptr;

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , spacer(new QWidget())
    , quickSpacerLeft(new QWidget())
    , quickSpacerRight(new QWidget())
    , timeEdit(new QTimeEdit()) {

    // set up ui
    ui->setupUi(this);
    ui->eventCombo->setModel(Event::instance());
    ui->eventCombo->setModelColumn(Event::Title);

    ui->teamCombo->setModel(Team::instance());
    ui->teamCombo->setModelColumn(Team::Title);

    // setup pixmaps
    ui->eventPixmap->setPixmap(QIcon::fromTheme("name").pixmap(16, 16));
    ui->teamPixmap->setPixmap(QIcon::fromTheme("teams").pixmap(16, 16));
    ui->findPixmap->setPixmap(QIcon::fromTheme("find").pixmap(16, 16));

    // setup task/logView
    ui->taskView->setModel(Task::instance());
    ui->taskView->setModelColumn(Task::Name);
    ui->taskView->setItemDelegate(new Delegate(ui->taskView));

    // bind event/team variables to comboBoxes
    Variable::instance()->bind("eventId", ui->eventCombo);
    Variable::instance()->bind("teamId", ui->teamCombo);

    // insert spacer
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBar->insertWidget(ui->actionAbout, spacer);

    // bind for sorting updates
    Variable::instance()->bind("sortByType", this, SLOT(setTaskFilter()));

    // set up completer
    completer.setModel(Task::instance());
    ui->findEdit->setCompleter(&completer);
    completer.setCompletionColumn(Task::Name);
    completer.setCaseSensitivity(Qt::CaseInsensitive);
    completer.setFilterMode(Qt::MatchContains);
    connect(ui->findEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        setTaskFilter(isComboModeActive(), currentComboId());
        setLock();
    });

    // set up secondary toolBar
    ui->quickBar->insertWidget(ui->actionLogTime, timeEdit);

    // position quickToolbar
    quickSpacerLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    quickSpacerRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->quickBar->insertWidget(ui->actionAddQuick, quickSpacerLeft);
    ui->quickBar->addWidget(quickSpacerRight);

    // currentTime button
    connect(ui->actionLogTime, &QAction::triggered, this, [this]() { timeEdit->setTime(QTime::currentTime()); });

    // done button
    connect(ui->actionDone, &QAction::triggered, this, [this]() {
        // here's what we do (MIND that at this point we see the filtered list):
        //   1) we get a list of all comboIds currently visible, such as
        //      task1   -1
        //      task2   13
        //      task3   -1
        //   2) here is what we do next:
        //      a) there are MULTIPLE tasks (as in >1) with valid comboId (not -1) - ALL OK
        //      b) there are NO tasks with valid comboId - ALL OK
        //      c) there is A SINGLE task with a valid comboId (all other's are set to -1):
        //         a single task log CANNOT have a comboId, therefore we must reset it to -1
        //         which is done via simple query
        //
        QList<Id> idList(qobject_cast<Delegate *>(ui->taskView->itemDelegate())->combos.values());
        idList.removeAll(Id::Invalid);

#ifdef QT_DEBUG
        qCDebug(Database_::Debug) << (idList.count() <= 1 ? "bad list" : "okay list") << idList.count();
#endif

        // do a simple query
        if (idList.count() == 1) {
            QSqlQuery query;
            query.exec(QString("UPDATE %1 SET %2=-1 WHERE %2=%3")
                           .arg(Log::instance()->tableName(), Log::instance()->fieldName(Log::Combo), QString::number(static_cast<int>(idList.first()))));

            // must perform SELECT!
            Log::instance()->select();
        }

        setTaskFilter();
    });

    // time updater
    connect(timeEdit, &QTimeEdit::timeChanged, this, [this](const QTime &time) {
        // check for valid event
        const Row event = currentEvent();
        if (event == Row::Invalid) return;

        // check for valid team
        const Row team = currentTeam();
        if (event == Row::Invalid) return;

        // check for valid time
        const QTime startTime = Event::instance()->startTime(event);
        const QTime finalTime = Event::instance()->finalTime(event).addSecs(10800);
        const QTime teamTime = Team::instance()->finishTime(team);

        if (time > finalTime) {
            timeEdit->setTime(finalTime);
            return;
        }

        if (time < startTime) {
            timeEdit->setTime(startTime);
            return;
        }

        // set new time if anything changed
        if (teamTime != time) Team::instance()->setFinishTime(currentTeam(), time);
    });

    // clear button
    connect(ui->clearButton, &QToolButton::pressed, this, [this]() { ui->findEdit->clear(); });

    // add to garbage man
    GarbageMan::instance()->add(this);

#ifndef QT_DEBUG
    ui->quickBar->removeAction(ui->actionExport_logs);
#endif

#ifdef __APPLE__
    auto menu = ui->menuBar->addMenu(tr("File"));
    menu->addAction(tr("Events"), [&]() { on_actionEvents_triggered(); })->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(tr("Teams"), [&]() { on_actionTeams_triggered(); })->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(tr("Tasks"), [&]() { on_actionTasks_triggered(); })->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(tr("Combos"), [&]() { on_actionCombos_triggered(); })->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(tr("Rankings"), [&]() { on_actionRankings_triggered(); })->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(tr("Preferences"), [&]() { on_actionSettings_triggered(); })->setMenuRole(QAction::PreferencesRole);
#endif

    // lock/unlock ui elements
    setLock();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // unbind variables
    Variable::instance()->unbind("eventId", ui->eventCombo);
    Variable::instance()->unbind("teamId", ui->teamCombo);

    // disconnect lambdas
    disconnect(ui->findEdit, SLOT(textChanged(QString)));
    disconnect(ui->actionLogTime, SLOT(triggered(bool)));
    disconnect(timeEdit, SLOT(timeChanged(QTime)));
    disconnect(ui->clearButton, SLOT(pressed()));
    disconnect(ui->actionDone, SLOT(triggered(bool)));

    // delete ui elements
    delete timeEdit;
    delete spacer;
    delete quickSpacerLeft;
    delete quickSpacerRight;
    delete ui;
}

/**
 * @brief MainWindow::currentEventRow
 * @return
 */
Row MainWindow::currentEvent() const {
    const int index = ui->eventCombo->currentIndex();

    // abort if database has not been initialized
    if (!Database::instance()->hasInitialised()) return Row::Invalid;

    if (index == -1) return Row::Invalid;

    return Event::instance()->row(index);
}

/**
 * @brief MainWindow::currentTeamRow
 * @return
 */
Row MainWindow::currentTeam() const {
    const int index = ui->teamCombo->currentIndex();

    // abort if database has not been initialized
    if (!Database::instance()->hasInitialised()) return Row::Invalid;

    if (index == -1) return Row::Invalid;

    return Team::instance()->row(index);
}

/**
 * @brief MainWindow::setCurrentTeam
 * @param id
 */
void MainWindow::setCurrentTeam(const Row &row) {
    if (row == Row::Invalid) return;

    ui->teamCombo->setCurrentIndex(static_cast<int>(row));
}

/**
 * @brief MainWindow::setCurrentEvent
 * @param id
 */
void MainWindow::setCurrentEvent(const Row &row) {
    if (row == Row::Invalid) return;

    ui->eventCombo->setCurrentIndex(static_cast<int>(row));
}

/**
 * @brief MainWindow::on_eventCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_eventCombo_currentIndexChanged(int index) {
    // abort if database has not been initialized
    if (!Database::instance()->hasInitialised()) {
        setLock();
        return;
    }

    // failsafe
    const Row row = Event::instance()->row(index);
    if (row == Row::Invalid) {
        Team::instance()->setFilter(Team::instance()->fieldName(Team::Event) + "=-1");

        setLock();
        return;
    }

    // filter tasks
    Team::instance()->setFilter(QString(Team::instance()->fieldName(Team::Event) + "=%1").arg(static_cast<int>(Event::instance()->id(row))));
    setTaskFilter();
}

/**
 * @brief MainWindow::on_teamCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_teamCombo_currentIndexChanged(int index) {
    // abort if database has not been initialized
    if (!Database::instance()->hasInitialised()) {
        setLock();
        return;
    }

    // reset item delegate if any
    if (ui->taskView->itemDelegate() != nullptr) {
        Delegate *delegate(qobject_cast<Delegate *>(ui->taskView->itemDelegate()));

        if (delegate != nullptr) delegate->reset();
    } else {
        setLock();
        return;
    }

    // update view
    ui->taskView->viewport()->update();

    // failsafe
    const Row team = Event::instance()->row(index);
    if (team == Row::Invalid) {
        setLock();

        ui->actionEvents->setDisabled(false);
        ui->actionTeams->setDisabled(currentEvent() == Row::Invalid);
        timeEdit->setTime(QTime());
        return;
    }

    // update time edit
    timeEdit->setTime(Team::instance()->finishTime(team));

    // reset task filter
    setTaskFilter();
}

/**
 * @brief MainWindow::on_actionEvents_triggered
 */
void MainWindow::on_actionEvents_triggered() {
    EditorDialog *editor(EditorDialog::instance());

    editor->container->clearSelection();
    editor->container->setModel(Event::instance());

    editor->container->hideColumn(Event::ID);
    editor->container->hideColumn(Event::API);
    editor->container->hideColumn(Event::Combo2);
    editor->container->hideColumn(Event::Combo3);
    editor->container->hideColumn(Event::Combo4);
    editor->container->hideColumn(Event::Lock);
    editor->container->hideColumn(Event::Min);
    editor->container->hideColumn(Event::Max);
    editor->container->hideColumn(Event::Penalty);
    editor->container->showColumn(Event::Title);
    editor->container->showColumn(Event::Start);
    editor->container->showColumn(Event::Finish);
    editor->container->showColumn(Event::Final);

    editor->container->resizeColumnsToContents();

    editor->setToolBar(EventToolBar::instance());
    editor->setWindowTitle(tr("Event manager"));
    editor->setWindowIcon(QIcon::fromTheme("ketone"));

    editor->show();
    EventToolBar::instance()->buttonTest();
    EventToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTeams_triggered
 */
void MainWindow::on_actionTeams_triggered() {
    EditorDialog *editor(EditorDialog::instance());

    editor->container->clearSelection();
    editor->container->setModel(Team::instance());

    // editor->container->setModelColumn( Team::Title );
    editor->container->hideColumn(Team::ID);
    editor->container->hideColumn(Team::Lock);
    editor->container->hideColumn(Team::Reviewer);
    editor->container->hideColumn(Team::Event);
    editor->container->showColumn(Team::Title);
    editor->container->showColumn(Team::Members);
    editor->container->showColumn(Team::Finish);

    editor->container->resizeColumnsToContents();

    editor->setToolBar(TeamToolBar::instance());
    editor->setWindowTitle(tr("Team manager"));
    editor->setWindowIcon(QIcon::fromTheme("teams"));

    editor->show();
    TeamToolBar::instance()->buttonTest();
    TeamToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTasks_triggered
 */
void MainWindow::on_actionTasks_triggered() {
    EditorDialog *editor(EditorDialog::instance());

    editor->container->clearSelection();
    editor->container->setModel(Task::instance());

    // editor->container->setModelColumn( Task::Name );
    editor->container->hideColumn(Task::ID);
    editor->container->hideColumn(Task::Style);
    editor->container->hideColumn(Task::Order_);
    editor->container->hideColumn(Task::Event);
    editor->container->hideColumn(Task::ComboID);
    editor->container->hideColumn(Task::Multi);

    editor->container->showColumn(Task::Name);
    editor->container->showColumn(Task::Points);
    editor->container->showColumn(Task::Mult);
    editor->container->showColumn(Task::Desc);

    editor->container->resizeColumnsToContents();

    editor->setToolBar(TaskToolBar::instance());
    editor->setWindowTitle(tr("Task manager"));
    editor->setWindowIcon(QIcon::fromTheme("tasks"));

    editor->show();
    TaskToolBar::instance()->buttonTest();
    TaskToolBar::instance()->show();

    // clear this, so that task editor is not empty
    ui->findEdit->clear();
}

/**
 * @brief MainWindow::setTaskFilter SQL-heavy task filter
 * @param filterByCombo
 * @param comboId
 */
void MainWindow::setTaskFilter(bool filterByCombo, const Id &comboId) {
    const bool sort = Variable::isEnabled("sortByType");
    const Id eventId = currentEvent() == Row::Invalid ? Id::Invalid : Event::instance()->id(currentEvent());
    const Id teamId = currentTeam() == Row::Invalid ? Id::Invalid : Team::instance()->id(currentTeam());
    const QString find(ui->findEdit->text());

    // make sure to store this variable
    m_comboMode = filterByCombo;
    m_currentCombo = comboId;

    // disable ui components
    setLock();

    // add/remove done action
    if (filterByCombo) ui->toolBar->insertAction(ui->actionEvents, ui->actionDone);
    else ui->toolBar->removeAction(ui->actionDone);

    // selects tasks from current event
    const QString eventFilter(QString("%1=%2").arg(Task::instance()->fieldName(Task::Event)).arg(static_cast<int>(eventId)));

    // selects matching tasks from quick search
    const QString comboFind(!filterByCombo ? "" : QString("OR %1=%2").arg(Log::instance()->fieldName(Log::Fields::Combo)).arg(static_cast<int>(comboId)));
    const QString findFilter(find.isEmpty() ? "" : QString("AND %1 LIKE '%%2%' %3 ").arg(Task::instance()->fieldName(Task::Name), find, comboFind));

    // selects tasks for combo mode
    const QString comboFilter(filterByCombo
            ? QString("and %1.%2 in "
                      "( SELECT %3 FROM %4 WHERE %5=%6 AND ( %7=%8 OR %7=-1 ) AND %9!=0 )")
                  .arg(Task::instance()->tableName(), Task::instance()->fieldName(Task::ID), Log::instance()->fieldName(Log::Task),
                      Log::instance()->tableName(), Log::instance()->fieldName(Log::Team), QString::number(static_cast<int>(teamId)),
                      Log::instance()->fieldName(Log::Combo), QString::number(static_cast<int>(comboId)), Log::instance()->fieldName(Log::Multi))
            : "");

    // orders tasks according to settings
    const QString comboOrder(QString("%1 DESC,").arg(Log::instance()->fieldName(Log::Fields::Combo)));
    const QString orderFilter(QString("ORDER BY %1 %2 ASC %3")
                                  .arg(!filterByCombo ? "" : comboOrder)
                                  .arg(sort ? Task::instance()->fieldName(Task::Style) : Task::instance()->fieldName(Task::Order_))
                                  .arg(sort ?
#ifdef SQLITE_CUSTOM
                                            QString(", %1 COLLATE localeCompare ASC")
#else
                                              QString(", %1 COLLATE NOCASE ASC")
#endif
                                                .arg(Task::instance()->fieldName(Task::Name))
                                            : ""));

    // put all filters together
    const QString filter(QString("%1 %2 %3 %4").arg(eventFilter, comboFilter, findFilter, orderFilter));

    // qDebug() << filter;

    // set filter
    Task::instance()->setFilter(filter);

    // scroll to the beginning
    ui->taskView->scrollToTop();
}

/**
 * @brief MainWindow::setUiLock
 * @param lock
 */
void MainWindow::setLock() {
    const bool comboMode = isComboModeActive();
    const bool noEvents = !ui->eventCombo->count();
    const bool noTeams = !ui->teamCombo->count();
    const bool noTasks = !Task::instance()->count();

    // lock common ui elements
    ui->actionTasks->setDisabled(comboMode || noEvents || noTeams);
    ui->actionRankings->setDisabled(comboMode || noEvents || noTeams);
    // ui->actionSettings->setDisabled( comboMode || noEvents || noTeams );
    ui->actionCombos->setDisabled(comboMode || noEvents || noTeams);
    ui->actionAddQuick->setDisabled(comboMode || noEvents || noTeams);
    ui->actionLogTime->setDisabled(comboMode || noEvents || noTeams);
    timeEdit->setDisabled(comboMode || noEvents || noTeams);
    ui->taskView->setDisabled(noEvents || noTeams || noTasks);

    // team disabler/enabler
    ui->teamCombo->setDisabled(comboMode || noEvents || noTeams);
    ui->actionTeams->setDisabled(comboMode || noEvents);

    // event disabler/enabler
    ui->eventCombo->setDisabled(comboMode || noEvents);
    ui->actionEvents->setDisabled(comboMode);
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    if (!isMaximized()) Variable::setCompressedByteArray("geometry/main", saveGeometry());

    // disallow closing when modal windows are open
    if (!isEnabled()) {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

/**
 * @brief MainWindow::showEvent
 * @param event
 */
void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    // restore main window geomery
    if (!Variable::value<QVariant>("geometry/main").isNull() && !isMaximized()) restoreGeometry(Variable::compressedByteArray("geometry/main"));
}

/**
 * @brief MainWindow::on_actionAddQuick_triggered
 */
void MainWindow::on_actionAddQuick_triggered() {
    TeamEdit *edit(TeamEdit::instance());

    EditorDialog::instance()->dock->setWidget(nullptr);
    edit->setParent(nullptr);
    edit->reset(false);
    edit->show();
    edit->setCurrentTime();
    edit->move(geometry().x() + geometry().width() / 2 - edit->geometry().width() / 2,
        geometry().y() + geometry().height() / 2 - edit->geometry().height() / 2);
}

/**
 * @brief MainWindow::on_actionRankings_triggered
 */
void MainWindow::on_actionRankings_triggered() { Rankings::instance()->show(); }

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() { Settings::instance()->show(); }

/**
 * @brief MainWindow::on_actionConsole_triggered
 */
void MainWindow::on_actionConsole_triggered() { Console::instance()->show(); }

/**
 * @brief MainWindow::on_actionCombos_triggered
 */
void MainWindow::on_actionCombos_triggered() { Combos::instance()->show(); }

/**
 * @brief MainWindow::on_actionAbout_triggered
 */
void MainWindow::on_actionAbout_triggered() { About(this).exec(); }

/**
 * @brief MainWindow::on_actionExport_logs_triggered
 */
void MainWindow::on_actionExport_logs_triggered() {
    QSqlQuery query;

    const Row team = currentTeam();
    if (team == Row::Invalid) return;

    // qDebug() << Team::instance()
    query.exec(QString("SELECT * from %1 where %2=%3")
                   .arg(Log::instance()->tableName(), Log::instance()->fieldName(Log::Team), QString::number(static_cast<int>(Team::instance()->id(team)))));

    QString path(QFileDialog::getSaveFileName(
        this, tr("Export logs to CSV format"), QDir::homePath() + "/" + Team::instance()->title(team) + ".csv", tr("CSV file (*.csv)")));

    // check for empty filenames
    if (path.isEmpty()) return;

    // add extension
    if (!path.endsWith(".csv")) path.append(".csv");

    // create file
    QFile csv(path);
    if (csv.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&csv);
        out << tr("Name;Style;Points").append("\n");

        while (query.next()) {
            const Id id = static_cast<Id>(query.value(Log::Task).toInt());
            if (id == Id::Invalid) continue;

            const Row row = Task::instance()->row(id);
            if (row == Row::Invalid) continue;

            out << QString("%1;%2;%3%4")
                       .arg(Task::instance()->name(row))
                       .arg(static_cast<int>(Task::instance()->style(row)))
                       .arg(query.value(Log::Multi).toInt())
                       .arg("\n");
        }
    }
    csv.close();
}
