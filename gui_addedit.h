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

#ifndef GUI_ADDEDIT_H
#define GUI_ADDEDIT_H

//
// includes
//
#include <QDialog>

//
// namespaces
//
namespace Ui {
class Gui_AddEdit;
}

//
// class Add/Edit dialog (gui)
//
class Gui_AddEdit : public QDialog {
    Q_OBJECT
    Q_ENUMS( Parent )
    Q_ENUMS( Mode )
    Q_PROPERTY( Parent dialogParent READ dialogParent )
    Q_PROPERTY( Mode mode READ mode )

public:
    enum Parent {
        NoParent = -1,
        EventDialog,
    };
    enum Mode {
        NoMode = -1,
        Add,
        Edit,
        Rename
    };
    explicit Gui_AddEdit( Parent = NoParent, Mode = NoMode, int id = -1, QWidget *parent = 0 );
    ~Gui_AddEdit();
    int id() const { return this->m_id; }
    Parent dialogParent() const { return this->m_dialogParent; }
    Mode mode() const { return this->m_mode; }

private slots:
    void on_acceptButton_clicked();

private:
    Ui::Gui_AddEdit *ui;
    int m_id;
    Parent m_dialogParent;
    Mode m_mode;
};

#endif // GUI_ADDEDIT_H
