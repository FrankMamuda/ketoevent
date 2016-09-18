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

#ifndef GUI_LICENSE_H
#define GUI_LICENSE_H

//
// includes
//
#include <QDialog>
#include <QApplication>

//
// namespaces
//
namespace Ui {
    class Gui_License;
}

/**
 * @brief The Gui_License class
 */
class Gui_License : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "License dialog" )

public:
    explicit Gui_License( QWidget *parent = 0 );
    ~Gui_License();

private slots:
    void on_aboutQtButton_clicked() { QApplication::aboutQt(); }

private:
    Ui::Gui_License *ui;
};

#endif // GUI_LICENSE_H
