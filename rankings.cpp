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
#include "rankings.h"
#include "event.h"
#include "log.h"
#include "main.h"
#include "mainwindow.h"
#include "rankingsmodel.h"
#include "task.h"
#include "team.h"
#include "ui_rankings.h"
#include "variable.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QTextStream>
#include <QtMath>
#ifdef XLSX_SUPPORT
#include <xlsxdocument.h>
#endif

// singleton
Rankings *Rankings::i = nullptr;

/**
 * @brief Rankings::Rankings
 * @param parent
 */
Rankings::Rankings() : ui(new Ui::Rankings), model(nullptr), proxyModel(nullptr) {
    // set up ui
    ui->setupUi(this);

    // setup pixmaps
    ui->teamPixmap->setPixmap(QIcon::fromTheme("teams").pixmap(16, 16));

    // make sure this window blocks other windows
    setWindowModality(Qt::ApplicationModal);

    // set up progressbar and team selector
    ui->progressBar->hide();
    ui->teamCombo->setModel(Team::instance());
    ui->teamCombo->setModelColumn(Team::Title);

    // bind currentTeam action to a variable
    // and repaint table when either team or currentTeam variable changes
    Variable::instance()->bind("rankingsCurrent", ui->actionCurrent);
    connect(ui->actionCurrent, SIGNAL(toggled(bool)), ui->tableView->viewport(), SLOT(repaint()));
    connect(ui->teamCombo, SIGNAL(currentIndexChanged(int)), ui->tableView->viewport(), SLOT(repaint()));

    // add to garbage man
    GarbageMan::instance()->add(this);

#ifdef XLSX_SUPPORT
    // export action (xlsx)
    ui->toolBar->addAction(QIcon::fromTheme("export"), tr("Export as xlsx"), [this]() {
        QString path(QFileDialog::getSaveFileName(this, tr("Export rankings to XLSX format"), QDir::homePath(), tr("XLSX spreadsheet (*.xlsx)")));

        // check for empty filenames
        if (path.isEmpty()) return;

        // add extension
        if (!path.endsWith(".xlsx")) path.append(".xlsx");

        // create file
        QXlsx::Document xlsx;

        QXlsx::Format boldFormat;
        boldFormat.setFontBold(true);

        int row = 1;
        xlsx.write(row, 1, tr("Team name"), boldFormat);
        xlsx.write(row, 2, tr("Tasks"), boldFormat);
        xlsx.write(row, 3, tr("Combos"), boldFormat);
        xlsx.write(row, 4, tr("Time"), boldFormat);
        xlsx.write(row, 5, tr("Penalty points"), boldFormat);
        xlsx.write(row, 6, tr("Total points"), boldFormat);
        row++;

        for (const TeamStatistics &team : std::as_const(list)) {
            xlsx.write(row, 1, team.title);
            xlsx.write(row, 2, team.completedTasks);
            xlsx.write(row, 3, team.combos);
            xlsx.write(row, 4, team.time);
            xlsx.write(row, 5, team.penalty);
            xlsx.write(row, 6, team.points);
            row++;
        }

        xlsx.setDocumentProperty("title", "Rankings");
        xlsx.setDocumentProperty("creator", "Ketoevent");
        xlsx.setDocumentProperty("description", "Exported with ketoevent via qtxlsx");

        xlsx.saveAs(path);
    });
#endif
}

/**
 * @brief Rankings::~Rankings
 */
Rankings::~Rankings() {
    Variable::instance()->unbind("rankingsCurrent", ui->actionCurrent);
    disconnect(ui->actionCurrent, SIGNAL(toggled(bool)));
    disconnect(ui->teamCombo, SIGNAL(currentIndexChanged(int)));

    delete ui;
}

/**
 * @brief Rankings::isDisplayingCurrentTeam
 * @return
 */
bool Rankings::isDisplayingCurrentTeam() const { return ui->actionCurrent->isChecked(); }

/**
 * @brief Rankings::on_actionUpdate_triggered
 */
void Rankings::on_actionUpdate_triggered() {
    int totalLogged = 0;

    // remove junk to make sure it does not affect results
    Log::instance()->removeOrphanedEntries();

    // set up and show progress bar
    ui->progressBar->setRange(0, Team::instance()->count());
    ui->progressBar->setValue(0);
    ui->progressBar->show();

    // create model if none available
    if (model == nullptr || proxyModel == nullptr) {
        model = new RankingsModel();
        proxyModel = new QSortFilterProxyModel();
        proxyModel->setSourceModel(model);
        proxyModel->setDynamicSortFilter(true);

        // setup table
        ui->tableView->setModel(proxyModel);
        ui->tableView->setWordWrap(true);
        ui->tableView->verticalHeader()->hide();
        ui->tableView->setSortingEnabled(true);
    }

    // clear any leftover results
    list.clear();
    model->reset();

    // get event related variables
    const Row event = MainWindow::instance()->currentEvent();
    if (event == Row::Invalid) return;

    const QTime eventStartTime(Event::instance()->startTime(event));
    const QTime eventFinishTime(Event::instance()->finishTime(event));
    const QTime eventFinalTime(Event::instance()->finalTime(event));
    const int penaltyPoints(Event::instance()->penalty(event));

    // go through team list (might seem a little less efficient than going through logs,
    // but in reality there is not that much of a performance penalty
    // team method also avoids unnecessary complexity over calculation by logs
    for (int team = 0; team < Team::instance()->count(); team++) {
        const Row teamRow = Team::instance()->row(team);

        if (teamRow == Row::Invalid) continue;

        TeamStatistics stats(Team::instance()->title(teamRow));
        QMap<Id, int> combos;
        QList<Id> dup;

        // update progress bar
        ui->progressBar->setValue(team);

        // go through logs
        for (int log = 0; log < Log::instance()->count(); log++) {
            const Row logRow = Log::instance()->row(log);
            const int value = Log::instance()->multiplier(logRow);

            if (logRow == Row::Invalid) continue;

            // abort on invalid log values or logs not related to the current team
            if (value == 0 || Team::instance()->id(teamRow) != Log::instance()->teamId(logRow)) continue;

            // get task related variables
            const Id taskId = Log::instance()->taskId(logRow);
            const Row task = Task::instance()->row(taskId);

            if (task == Row::Invalid) continue;

            const Task::Types type = Task::instance()->type(task);

            // test for duplicates
            if (dup.contains(taskId)) continue;
            dup << taskId;

            // increment completed tasks
            stats.completedTasks++;

            // calculate points form completed tasks
            stats.points += Task::instance()->points(task) * ((type == Task::Types::Multi) ? value : 1);

            // build combo map
            const Id comboId = Log::instance()->comboId(logRow);
            if (!combos.contains(comboId) && comboId > Id::Invalid) combos[comboId] = 0;

            if (comboId > Id::Invalid) {
                combos[comboId] = combos[comboId] + 1;
                stats.comboTasks += 1;
            }

#ifdef KK6_SPECIAL
            // NOTE: hardcoded
            // special event-related points
            const QString taskName(Task::instance()->name(task));
            if (!QString::compare(taskName, "Papilduzdevums")) stats.specialPoints1 = value;
            else if (!QString::compare(taskName, "FTF")) stats.specialPoints0 = value;
#endif
        }

        // store combo count
        stats.combos = static_cast<int>(combos.count());

        // calculate bonus points from combos
        for (const int count : std::as_const(combos)) {
            if (count == 2) stats.points += EventTable::DefaultComboOfTwo;

            if (count == 3) stats.points += EventTable::DefaultComboOfThree;

            if (count >= 4) stats.points += EventTable::DefaultComboOfFourAndMore;
        }

        // calculate penalty points
        const int overTime = eventFinishTime.secsTo(Team::instance()->finishTime(teamRow)) / 60 + 1;
        stats.time = eventStartTime.secsTo(Team::instance()->finishTime(teamRow)) / 60 + 1;
        if (overTime > 0) {
            stats.penalty = penaltyPoints * overTime;
            stats.points -= stats.penalty;
            stats.points = qMax(stats.points, 0);
        }
        const int penaltyTime = eventFinishTime.secsTo(eventFinalTime);
        if (overTime > penaltyTime || Team::instance()->finishTime(teamRow) >= eventFinalTime) stats.points = 0;

        // add team stats to list
        list << stats;
        totalLogged += stats.points;
    }

    // update model
    model->reset();

    // hide progress bar
    ui->progressBar->hide();

    // sort by points
    proxyModel->sort(RankingsModel::Points, Qt::DescendingOrder);

    // scale window to contents
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    // calculate rank
    // NOTE: a really dumb way to do it
    QMultiMap<int, int> map;

    int y = 0;
    for (const TeamStatistics &stats : std::as_const(list)) {
        map.insert(stats.points, y);
        y++;
    }

    QList<int> points(map.uniqueKeys());
    std::sort(points.begin(), points.end(), std::greater<int>());
    y = 1;

    for (const int p : std::as_const(points)) {
        QList<int> indices(map.values(p));
        for (int index : indices) {
            TeamStatistics stats = list.at(index);
            stats.rank = y;
            list.replace(index, stats);
        }
        y++;
    }

    // total teams
    ui->totalTeamsEdit->setText(QString::number(Team::instance()->count()));

    // total members
    int members = 0;
    for (int y = 0; y < Team::instance()->count(); y++) members += Team::instance()->members(Team::instance()->row(y));
    ui->totalMembersEdit->setText(QString::number(members));

    // total logged points
    ui->totalLoggedEdit->setText(QString::number(totalLogged));

    // total tasks
    ui->totalTasksEdit->setText(QString::number(Task::instance()->count()));

    // total points
    int totalPoints = 0;
    for (int y = 0; y < Task::instance()->count(); y++) {
        const Row row = Task::instance()->row(y);
        const Task::Types type = Task::instance()->type(row);

        totalPoints += (Task::instance()->points(row) * (type == Task::Types::Multi ? Task::instance()->multi(row) : 1));
    }
    int comboPoints = static_cast<int>(qFloor(static_cast<qreal>(Task::instance()->count()) / 4))
        * Event::instance()->comboOfFourPlus(MainWindow::instance()->currentEvent());
    switch (Task::instance()->count() % 4) {
    case 3: comboPoints += Event::instance()->comboOfThree(MainWindow::instance()->currentEvent()); break;
    case 2: comboPoints += Event::instance()->comboOfTwo(MainWindow::instance()->currentEvent()); break;
    }
    ui->totalPointsEdit->setText(tr("%1 (%2+%3)").arg(totalPoints + comboPoints).arg(totalPoints).arg(comboPoints));

    // # of tasks completed
    int numTasksCompleted = 0;
    QSqlQuery query;
    query.exec(QString("SELECT COUNT(*) from %1 WHERE %2>0 GROUP BY %3")
                   .arg(Log::instance()->tableName(), Log::instance()->fieldName(Log::Multi), Log::instance()->fieldName(Log::Task)));
    while (query.next()) numTasksCompleted += query.value(0).toInt();
    ui->completedEdit->setText(QString::number(numTasksCompleted));
}

/**
 * @brief Rankings::showEvent
 * @param event
 */
void Rankings::showEvent(QShowEvent *event) {
    ModalWindow::showEvent(event);

    // scale window to contents
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();

    // set current team
    ui->teamCombo->setCurrentIndex(static_cast<int>(MainWindow::instance()->currentTeam()));

    // restore main window geomery
    if (!Variable::value<QVariant>("geometry/rankings").isNull() && !isMaximized())
        restoreGeometry(Variable::compressedByteArray("geometry/rankings"));
}

/**
 * @brief Rankings::closeEvent
 * @param event
 */
void Rankings::closeEvent(QCloseEvent *event) {
    if (!isMaximized()) Variable::setCompressedByteArray("geometry/rankings", saveGeometry());

    ModalWindow::closeEvent(event);
}

/**
 * @brief Rankings::on_actionExport_triggered
 */
void Rankings::on_actionExport_triggered() {
    QString path(QFileDialog::getSaveFileName(this, tr("Export statistics to CSV format"), QDir::homePath(), tr("CSV file (*.csv)")));

    // check for empty filenames
    if (path.isEmpty()) return;

    // add extension
    if (!path.endsWith(".csv")) path.append(".csv");

    // create file
    QFile csv(path);

    if (csv.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&csv);
        out << tr("Team name;Tasks;Combos;Time;Penalty points;Total points").append("\n");

        for (const TeamStatistics &team : std::as_const(list)) {
            out << QString("%1;%2;%3;%4;%5;%6%7")
                       .arg(team.title)
                       .arg(team.completedTasks)
                       .arg(team.combos)
                       .arg(team.time)
                       .arg(team.penalty)
                       .arg(team.points)
                       .arg("\n");
        }
    }
    csv.close();
}
