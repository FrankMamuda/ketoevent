/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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
#include "main.h"
#include "eventtoolbar.h"
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
EditorDialog::EditorDialog() :
    container( nullptr ),
    dock( nullptr ),
    toolBar( nullptr ),
    ui( new Ui::EditorDialog )
{
    // set up ui
    this->ui->setupUi( this );
    this->container = this->ui->tableView;
    this->container->verticalHeader()->hide();
    this->dock = this->ui->dockWidget;
    this->hideDock();

    // connect close button
    this->connect( this->ui->buttonClose, &QPushButton::clicked, [ this ] () {
        this->close();
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief EditorDialog::~EditorDialog
 */
EditorDialog::~EditorDialog() {
    this->disconnect( this->ui->buttonClose, SIGNAL( clicked()));
    delete this->ui;
}

/**
 * @brief EditorDialog::isDockVisible
 * @return
 */
bool EditorDialog::isDockVisible() const {
    return this->dock->isVisible();
}

/**
 * @brief EditorDialog::showDock
 */
void EditorDialog::showDock( QWidget *contents , const QString &title ) {
    this->dock->setWidget( contents );
    this->dock->setWindowTitle( title );
    this->dock->show();
    this->ui->buttonClose->hide();
    this->container->setDisabled( true );

    if ( this->toolBar != nullptr )
        this->toolBar->setDisabled( true );
}

/**
 * @brief EditorDialog::hideDock
 */
void EditorDialog::hideDock() {
    this->dock->hide();
    this->ui->buttonClose->show();
    this->container->setEnabled( true );

    if ( this->toolBar != nullptr )
        this->toolBar->setEnabled( true );
}

/**
 * @brief EditorDialog::setToolBar
 * @param toolBar
 */
void EditorDialog::setToolBar( QToolBar *widget ) {
    this->removeToolBar( this->toolBar );
    this->toolBar = widget;
    this->addToolBar( Qt::TopToolBarArea, this->toolBar );
}

/**
 * @brief EditorDialog::showEvent
 * @param event
 */
void EditorDialog::showEvent( QShowEvent *event ) {
    ModalWindow::showEvent( event );

    if ( !this->isMaximized()) {
        if ( this->toolBar == EventToolBar::instance() && !Variable::value<QVariant>( "geometry/events" ).isNull())
            this->restoreGeometry( Variable::compressedByteArray( "geometry/events" ));

        if ( this->toolBar == TaskToolBar::instance() && !Variable::value<QVariant>( "geometry/tasks" ).isNull())
            this->restoreGeometry( Variable::compressedByteArray( "geometry/tasks" ));

        if ( this->toolBar == TeamToolBar::instance() && !Variable::value<QVariant>( "geometry/teams" ).isNull())
            this->restoreGeometry( Variable::compressedByteArray( "geometry/teams" ));
    }
}

/**
 * @brief EditorDialog::closeEvent
 * @param event
 */
void EditorDialog::closeEvent( QCloseEvent *event ) {
    if ( !this->isMaximized()) {
        if ( this->toolBar == EventToolBar::instance())
            Variable::setCompressedByteArray( "geometry/events", this->saveGeometry());

        if ( this->toolBar == TaskToolBar::instance())
            Variable::setCompressedByteArray( "geometry/tasks", this->saveGeometry());

        if ( this->toolBar == TeamToolBar::instance())
            Variable::setCompressedByteArray( "geometry/teams", this->saveGeometry());
    }

    this->hideDock();
    this->disconnect( this->container, SIGNAL( clicked( QModelIndex )));
    ModalWindow::closeEvent( event );
}
