/*
===========================================================================
Copyright (C) 2013-2014 Avotu Briezhaudzetava

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef GUI_CONSOLE_H
#define GUI_CONSOLE_H

//
// includes
//
#include <QDialog>
#include <QMouseEvent>

//
// namespace: Ui
//
namespace Ui {
class Gui_Console;
}

//
// class: Gui_Console
//
class Gui_Console : public QDialog {
    Q_OBJECT

public:
    explicit Gui_Console(QWidget *parent = 0);
    ~Gui_Console();

public slots:
    void print( QString msg );

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );

private:
    Ui::Gui_Console *ui;
    QPoint m_windowPos;
};

#endif // GUI_CONSOLE_H
