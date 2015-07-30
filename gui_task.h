/*
===========================================================================
Copyright (C) 2013-2015 Avotu Briezhaudzetava

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

#ifndef GUI_TASK_H
#define GUI_TASK_H

//
// includes
//
#include <QDialog>
#include <QKeyEvent>
#include "gui_task.h"
#include "gui_dialog.h"
#include "gui_tasklistmodel.h"

//
// namespaces
//
namespace Ui {
    class Gui_Task;
}

//
// class: Gui_Task
//
class Gui_Task : public Gui_Dialog {
    Q_OBJECT
    Q_ENUMS( AddEditState )
    Q_ENUMS( MoveDirection )
    Q_ENUMS( ViewState )
    Q_PROPERTY( AddEditState state READ state WRITE setState )
    Q_CLASSINFO( "description", "Task handling dialog" )
    Q_PROPERTY( int currentMatch READ currentMatch WRITE setCurrentMatch )

public:
    // widget states
    enum AddEditState {
        NoState = -1,
        Add,
        Edit
    };
    enum MoveDirection {
        NoWhere = -1,
        Up,
        Down
    };
    enum ViewState {
        NoViewState = -1,
        Enabled,
        Disabled
    };

    explicit Gui_Task( QWidget * = 0 );
    ~Gui_Task();
    AddEditState state() const { return this->m_state; }
    int currentMatch() const { return this->m_currentMatch; }

private slots:
    void on_doneButton_clicked();
    void on_cancelButton_clicked() { this->toggleAddEditWidget( NoState ); }
    void setState( AddEditState state ) { this->m_state = state; }
    void toggleView( ViewState viewState = Enabled );
    void on_addEditWidget_visibilityChanged( bool visible ) { if ( !visible ) this->toggleView(); }
    void on_clearText_clicked();
    void findTask();
    void on_findTask_textChanged(const QString &arg1);
    void on_taskType_currentIndexChanged(int index);
    void changeTaskType( TaskEntry::Types type );
    void toggleAddEditWidget( AddEditState state );

    // search
    void setCurrentMatch( int match = 0 ) { this->m_currentMatch = match; }

    // actions
    void on_actionAdd_triggered() { this->toggleAddEditWidget( Add ); }
    void on_actionEdit_triggered() { this->toggleAddEditWidget( Edit ); }
    void on_actionRemove_triggered();
    void on_actionSort_triggered();
    void on_actionMoveUp_triggered() { this->move( Up ); }
    void on_actionMoveDown_triggered() { this->move( Down ); }

private:
    Ui::Gui_Task *ui;
    AddEditState m_state;
    Gui_TaskListModel *listModelPtr;
    int m_currentMatch;
    void move( MoveDirection direction );

protected:
    virtual void keyPressEvent( QKeyEvent * );
};

#endif // GUI_TASK_H

