/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

#ifndef GUI_DESCRIPTION_H
#define GUI_DESCRIPTION_H

//
// includes
//
#include <QDialog>
#include "task.h"

//
// namespace: Ui
//
namespace Ui {
class Gui_Description;
}

/**
 * @brief The Gui_Description class
 */
class Gui_Description : public QDialog {
    Q_OBJECT

public:
    explicit Gui_Description( Task *taskPtr = 0, QWidget *parent = 0 );
    ~Gui_Description();

private:
    Ui::Gui_Description *ui;
};

#endif // GUI_DESCRIPTION_H
