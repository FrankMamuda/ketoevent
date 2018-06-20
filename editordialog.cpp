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
#include "editordialog.h"
#include "mainwindow.h"
#include "ui_editordialog.h"
#include <QCommonStyle>
#include <QDebug>

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
    QCommonStyle style;

    qDebug() << "new edit dialog";

    // set up ui
    this->ui->setupUi( this );
    this->ui->buttonClose->setIcon( style.standardIcon( QStyle::SP_DialogCloseButton ));
    this->container = this->ui->listView;
    this->dock = this->ui->dockWidget;
    this->hideDock();

    // connect close button
    this->connect( this->ui->buttonClose, &QPushButton::clicked, [ this ] () {
        this->close();
    } );
}

/**
 * @brief EditorDialog::~EditorDialog
 */
EditorDialog::~EditorDialog() {
    qDebug() << "delete edit dialog";
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
 * @brief EditorDialog::closeEvent
 * @param event
 */
void EditorDialog::closeEvent( QCloseEvent *event ) {
    qDebug() << "close edit dialog";
    this->disconnect( this->container, SIGNAL( clicked( QModelIndex )));
    ModalWindow::closeEvent( event );
}
