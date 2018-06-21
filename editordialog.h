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

#pragma once

//
// includes
//
#include "modalwindow.h"
#include <QCloseEvent>
#include <QDockWidget>
#include <QListView>
#include <QMainWindow>
#include <QToolBar>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class EditorDialog;
}

/**
 * @brief The EditorDialog class
 */
class EditorDialog final : public ModalWindow {
    Q_OBJECT
    Q_DISABLE_COPY( EditorDialog )

public:
    static EditorDialog *instance() { static EditorDialog *instance = new EditorDialog(); return instance; }
    virtual ~EditorDialog();
    QListView *container;
    QDockWidget *dock;
    bool isDockVisible() const;

public slots:
    void showDock( QWidget *contents = nullptr, const QString &title = QString());
    void hideDock();
    void setToolBar( QToolBar *widget );

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    explicit EditorDialog();
    QToolBar *toolBar;
    Ui::EditorDialog *ui;
};
