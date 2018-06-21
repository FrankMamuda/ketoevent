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

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    this->ui->setupUi( this );

    this->ui->comboEvent->setModel( Event::instance());
    this->ui->comboEvent->setModelColumn( Event::Title );

    this->ui->comboTeam->setModel( Team::instance());
    this->ui->comboTeam->setModelColumn( Team::Title );

    this->ui->taskView->setModel( Task::instance());
    this->ui->taskView->setModelColumn( Task::Name );
    this->ui->taskView->setItemDelegate( new LogDelegate( this->ui->taskView ));
    //this->ui->taskView->setParent( this );

    Variable::instance()->bind( "eventId", this->ui->comboEvent );
    this->ui->comboTeam->setObjectName( "One" );
    Variable::instance()->bind( "teamId", this->ui->comboTeam );

    // insert spacer
    QWidget *spacer( new QWidget());
    spacer->setSizePolicy( QSizePolicy::Expanding,QSizePolicy::Preferred );
    this->ui->toolBar->insertWidget( this->ui->actionSettings, spacer );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    delete this->ui;
}

/**
 * @brief MainWindow::currentEventId
 * @return
 */
Id MainWindow::currentEventId() const {
    return Event::instance()->id( this->ui->comboEvent->currentIndex());
}

/**
 * @brief MainWindow::currentTeamId
 * @return
 */
Id MainWindow::currentTeamId() const {
    if ( !Database::instance()->hasInitialised())
        return Id::fromInteger( -1 );

    return Team::instance()->id( this->ui->comboTeam->currentIndex());
}

/**
 * @brief MainWindow::on_buttonAdd_clicked
 */
void MainWindow::on_buttonAdd_clicked() {
    bool ok;
    QString text;


    text = QInputDialog::getText( this, this->tr( "Add an event" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok );
    if ( ok && !text.isEmpty())
        Event::instance()->add( text );
}

/**
 * @brief MainWindow::on_buttonRemove_clicked
 */
void MainWindow::on_buttonRemove_clicked() {
    Event::instance()->remove( this->ui->comboEvent->currentIndex());
}

/**
 * @brief MainWindow::on_buttonRename_clicked
 */
void MainWindow::on_buttonRename_clicked() {
    bool ok;
    QString text( QInputDialog::getText( this, this->tr( "Rename event" ), this->tr( "Title:" ), QLineEdit::Normal, "", &ok ));

    if ( ok && !text.isEmpty())
        Event::instance()->setTitle( this->ui->comboEvent->currentIndex(), text );
}

/**
 * @brief MainWindow::on_comboEvent_currentIndexChanged
 * @param index
 */
void MainWindow::on_comboEvent_currentIndexChanged( int index ) {
    if ( !Database::instance()->hasInitialised())
        return;

    Team::instance()->setFilter( QString( "eventId=%1" ).arg( Event::instance()->id( index ).value()));
    Task::instance()->setFilter( QString( "eventId=%1" ).arg( Event::instance()->id( index ).value()));
}

/**
 * @brief MainWindow::on_comboTeam_currentIndexChanged
 * @param index
 */
void MainWindow::on_comboTeam_currentIndexChanged( int index ) {
    if ( !Database::instance()->hasInitialised())
        return;

    // NOTE: is this really needed?
    Log::instance()->setFilter( QString( "teamId=%1" ).arg( Team::instance()->id( index ).value()));
    this->ui->taskView->viewport()->update();    
    this->ui->taskView->setEnabled( index == -1 ? false : true );
}

/**
 * @brief MainWindow::on_actionEditor_triggered
 */
void MainWindow::on_actionEditor_triggered() {
    EditorDialog::instance()->show();
    EditorDialog::instance()->container->setModel( Team::instance());
    EditorDialog::instance()->container->setModelColumn( Team::Title );
    EditorDialog::instance()->setToolBar( TeamToolBar::instance());
    EditorDialog::instance()->setWindowTitle( this->tr( "Team manager" ));
    TeamToolBar::instance()->show();
}

/**
 * @brief MainWindow::on_actionTasks_triggered
 */
void MainWindow::on_actionTasks_triggered() {
    EditorDialog::instance()->show();
    EditorDialog::instance()->container->setModel( Task::instance());
    EditorDialog::instance()->container->setModelColumn( Task::Name );
    EditorDialog::instance()->setToolBar( TaskToolBar::instance());
    EditorDialog::instance()->setWindowTitle( this->tr( "Task manager" ));
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
