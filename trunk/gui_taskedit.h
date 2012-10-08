/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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

#ifndef GUI_TASKEDIT_H
#define GUI_TASKEDIT_H

//
// includes
//
#include <QDialog>
#include <QKeyEvent>
#include "gui_tasklistmodel.h"

//
// namespaces
//
namespace Ui {
    class Gui_TaskEdit;
}

//
// class:Gui_TaskEdit
//
class Gui_TaskEdit : public QDialog {
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

    explicit Gui_TaskEdit( QWidget * = 0 );
    ~Gui_TaskEdit();
    void toggleAddEditWidget( AddEditState state );
    AddEditState state() const { return this->m_state; }

private slots:
    void on_removeTaskButton_clicked();
    void on_addTaskButton_clicked() { this->toggleAddEditWidget( Add ); }
    void on_doneButton_clicked();
    void on_editTaskButton_clicked() { this->toggleAddEditWidget( Edit ); }
    void on_cancelButton_clicked() { this->toggleAddEditWidget( NoState ); }
    void setState( AddEditState state ) { this->m_state = state; }
    void enableView();
    void on_addEditWidget_visibilityChanged( bool visible ) { if ( !visible ) this->enableView(); }
    void on_clearText_clicked();
    void findTask();    
    void on_findTask_textChanged(const QString &arg1);
    void on_taskType_currentIndexChanged(int index);
    void changeTaskType( App_TaskEntry::Types type );

private:
    Ui::Gui_TaskEdit *ui;
    AddEditState m_state;
    Gui_TaskListModel *listModelPtr;
    int currentMatch;

protected:
    virtual void keyPressEvent( QKeyEvent * );
};


#endif // GUI_TASKEDIT_H
