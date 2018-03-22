/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "teamlistmodel.h"
#include "dialog.h"

//
// namespaces
//
namespace Ui {
    class TeamDialog;
}

/**
 * @brief The TeamDialog class
 */
class TeamDialog : public Dialog {
    Q_OBJECT
    Q_ENUMS( AddEditState )
    Q_PROPERTY( AddEditState state READ state WRITE setState )
    Q_PROPERTY( int lastId READ lastId WRITE setLastId RESET resetLastId )
    Q_CLASSINFO( "description", "Team management dialog" )

public:
    // widget states
    enum AddEditState {
        NoState = -1,
        Add,
        Edit,
        AddQuick
    };

    explicit TeamDialog( QWidget * = 0 );
    ~TeamDialog();
    void toggleAddEditWidget( AddEditState state );
    AddEditState state() const { return this->m_state; }
    int lastId() const { return this->m_lastId; }

private slots:
    void on_doneButton_clicked();
    void on_cancelButton_clicked() { this->toggleAddEditWidget( NoState ); }
    void setState( AddEditState state ) { this->m_state = state; }
    void enableView();
    void on_addEditWidget_visibilityChanged( bool visible ) { if ( !visible ) this->enableView(); }
    void setLastId( int id ) { this->m_lastId = id; }
    void resetLastId() { this->m_lastId = -1; }
    void on_reviewerButton_clicked();
    void on_actionAdd_triggered() { this->toggleAddEditWidget( Add ); }
    void on_actionRemove_triggered();
    void on_actionEdit_triggered() { this->toggleAddEditWidget( Edit ); }

protected:
    virtual void closeEvent( QCloseEvent * );

private:
    Ui::TeamDialog *ui;
    AddEditState m_state;
    TeamListModel *listModelPtr;
    int m_lastId;
};
