/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//
#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "event.h"
#include "task.h"
#include "team.h"
#include "log.h"
#include "logdelegate.h"
#include <QDebug>
#include "editordialog.h"
#include "tasktoolbar.h"
#include "teamtoolbar.h"
#include "rankings.h"
#include "settings.h"
#include "variable.h"
#include "database.h"
#include "console.h"
#include "combos.h"
#include "main.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), filter( new QSortFilterProxyModel()) {
    // set up ui
    this->ui->setupUi( this );
    this->ui->eventCombo->setModel( Event::instance());
    this->ui->eventCombo->setModelColumn( Event::Title );

    this->ui->teamCombo->setModel( Team::instance());
    this->ui->teamCombo->setModelColumn( Team::Title );

    // setup task/logView
    // NOTE: must handle proxy model indexes properly!
    this->filter->setFilterKeyColumn( Task::Name );
    this->filter->setFilterCaseSensitivity( Qt::CaseInsensitive );
    this->filter->setSourceModel( Task::instance());
    this->ui->taskView->setModel( this->filter );
    this->ui->taskView->setModelColumn( Task::Name );
    this->ui->taskView->setItemDelegate( new LogDelegate( this->ui->taskView ));

    // bind event/team variables to comboBoxes
    Variable::instance()->bind( "eventId", this->ui->eventCombo );
    Variable::instance()->bind( "teamId", this->ui->teamCombo );

    // insert spacer
    // TODO: delete me?
    QWidget *spacer( new QWidget());
    spacer->setSizePolicy( QSizePolicy::Expanding,QSizePolicy::Preferred );
    this->ui->toolBar->insertWidget( this->ui->actionSettings, spacer );

    // bind for sorting updates
    Variable::instance()->bind( "sortByType", this, SLOT( updateTasks()));

    // set up completer
    this->completer.setModel( Task::instance());
    this->ui->findEdit->setCompleter( &this->completer );
    this->completer.setCompletionColumn( Task::Name );
    this->completer.setCaseSensitivity( Qt::CaseInsensitive );
    this->connect( this->ui->findEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        qobject_cast<QSortFilterProxyModel*>( this->ui->taskView->model())->setFilterRegExp( text );
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    Variable::instance()->unbind( "eventId", this->ui->eventCombo );
    Variable::instance()->unbind( "teamId", this->ui->teamCombo );
    delete this->filter;
    delete this->ui;
}

/**
 * @brief MainWindow::currentEventId
 * @return
 */
Id MainWindow::currentEventId() const {
    return Event::instance()->id( this->ui->eventCombo->currentIndex());
}

/**
 * @brief MainWindow::currentTeamId
 * @return
 */
Id MainWindow::currentTeamId() const {
    if ( !Database::instance()->hasInitialised())
        return Id::Invalid;

    return Team::instance()->id( this->ui->teamCombo->currentIndex());
}

/**
 * @brief MainWindow::on_buttonAdd_clicked
 */
/*void MainWindow::on_buttonAdd_clicked() {
    bool ok;
    QString text;


    text = QInputDialog::getText( this, this->tr( "Add an event" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok );
    if ( ok && !text.isEmpty())
        Event::instance()->add( text );
}*/

/**
 * @brief MainWindow::on_eventCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_eventCombo_currentIndexChanged( int index ) {
    if ( !Database::instance()->hasInitialised())
        return;

    Team::instance()->setFilter( QString( "eventId=%1" ).arg( static_cast<int>( Event::instance()->id( index ))));
    this->updateTasks();
}

/**
 * @brief MainWindow::on_teamCombo_currentIndexChanged
 * @param index
 */
void MainWindow::on_teamCombo_currentIndexChanged( int index ) {
    if ( !Database::instance()->hasInitialised())
        return;

    this->ui->taskView->viewport()->update();
    this->ui->taskView->setEnabled( index == -1 ? false : true );
}

/**
 * @brief MainWindow::on_actionTeams_triggered
 */
void MainWindow::on_actionTeams_triggered() {
    EditorDialog *editor( EditorDialog::instance());

    editor->show();
    editor->container->clearSelection();
    editor->container->setModel( Team::instance());
    editor->container->setModelColumn( Team::Title );
    editor->setToolBar( TeamToolBar::instance());
    editor->setWindowTitle( this->tr( "Team manager" ));
    editor->setWindowIcon( QIcon( ":/icons/teams" ));

    TeamToolBar::instance()->buttonTest();
    TeamToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTasks_triggered
 */
void MainWindow::on_actionTasks_triggered() {
    EditorDialog *editor( EditorDialog::instance());

    editor->show();
    editor->container->clearSelection();
    editor->container->setModel( Task::instance());
    editor->container->setModelColumn( Task::Name );
    editor->setToolBar( TaskToolBar::instance());
    editor->setWindowTitle( this->tr( "Task manager" ));
    editor->setWindowIcon( QIcon( ":/icons/tasks" ));

    TaskToolBar::instance()->buttonTest();
    TaskToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionRankings_triggered
 */
void MainWindow::on_actionRankings_triggered() {
    Rankings::instance()->show();    
}

/**
 * @brief MainWindow::on_actionSettings_triggered
 */
void MainWindow::on_actionSettings_triggered() {
    Settings::instance()->show();
}

/**
 * @brief MainWindow::on_actionConsole_triggered
 */
void MainWindow::on_actionConsole_triggered() {
    Console::instance()->show();
}

/**
 * @brief MainWindow::updateTasks
 */
void MainWindow::updateTasks() {
    // NOTE: this could also be done through proxy model
    if ( Variable::instance()->isEnabled( "sortByType" ))
        Task::instance()->setFilter( QString( "eventId=%1 order by %2, %3 asc" ).arg( static_cast<int>( Event::instance()->id( this->ui->eventCombo->currentIndex()))).arg( Task::instance()->fieldName( Task::Type )).arg( Task::instance()->fieldName( Task::Name )));
    else
        Task::instance()->setFilter( QString( "eventId=%1 order by %2 asc" ).arg( static_cast<int>( Event::instance()->id( this->ui->eventCombo->currentIndex()))).arg( Task::instance()->fieldName( Task::Order )));
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent( QCloseEvent *event ) {
    // disallow closing when modal windows are open
    if ( !this->isEnabled()) {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent( event );
}

/**
 * @brief MainWindow::on_actionCombos_triggered
 */
void MainWindow::on_actionCombos_triggered() {
    Combos::instance()->show();
}
