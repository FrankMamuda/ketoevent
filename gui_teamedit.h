/*
===========================================================================
Copyright (C) 2013 Avotu Briezhaudzetava

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

#ifndef GUI_TEAMEDIT_H
#define GUI_TEAMEDIT_H

//
// includes
//
#include <QtWidgets/QDialog>
#include <QCloseEvent>
#include "gui_teamlistmodel.h"

//
// namespaces
//
namespace Ui {
    class Gui_TeamEdit;
}

//
// class: Gui_TeamEdit
//
class Gui_TeamEdit : public QDialog {
    Q_OBJECT
    Q_ENUMS( AddEditState )
    Q_PROPERTY( AddEditState state READ state WRITE setState )

public:
    // widget states
    enum AddEditState {
        NoState = -1,
        Add,
        Edit
    };

    explicit Gui_TeamEdit( QWidget * = 0 );
    ~Gui_TeamEdit();
    void toggleAddEditWidget( AddEditState state );
    AddEditState state() const { return this->m_state; }

private slots:
    void on_removeTeamButton_clicked();
    void on_addTeamButton_clicked() { this->toggleAddEditWidget( Add ); }
    void on_doneButton_clicked();
    void on_editTeamButton_clicked() { this->toggleAddEditWidget( Edit ); }
    void on_cancelButton_clicked() { this->toggleAddEditWidget( NoState ); }
    void setState( AddEditState state ) { this->m_state = state; }
    void enableView();
    void on_addEditWidget_visibilityChanged( bool visible ) { if ( !visible ) this->enableView(); }

private:
    Ui::Gui_TeamEdit *ui;
    AddEditState m_state;
    Gui_TeamListModel *listModelPtr;
};


#endif // GUI_TEAMEDIT_H
