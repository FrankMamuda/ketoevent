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

#pragma once

/*
 * includes
 */
#include "modalwindow.h"
#include <QCloseEvent>
#include <QDockWidget>
#include <QTableView>
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
    Q_DISABLE_COPY_MOVE( EditorDialog )

public:
    static EditorDialog *instance() { if ( EditorDialog::i == nullptr ) EditorDialog::i = new EditorDialog(); return EditorDialog::i; }
    ~EditorDialog() override;
    QTableView *container;
    QDockWidget *dock;
    [[nodiscard]] bool isDockVisible() const;

public slots:
    void showDock( QWidget *contents = nullptr, const QString &title = QString());
    void hideDock();
    void setToolBar( QToolBar *widget );

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    static EditorDialog *i;
    explicit EditorDialog();
    QToolBar *toolBar;
    Ui::EditorDialog *ui;
};
