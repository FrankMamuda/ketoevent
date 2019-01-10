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
#include <QDialog>
#include "task.h"
#include "ui_description.h"

//
// namespace: Ui
//
namespace Ui {
class Description;
}

/**
 * @brief The Description class
 */
class Description : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Description
     * @param task
     * @param parent
     */
    explicit Description( Task *task = 0, QWidget *parent = 0 ) : QDialog( parent ), ui( new Ui::Description ) {
        this->ui->setupUi( this );
        this->setWindowFlags( this->windowFlags() | Qt::Tool );

        if ( task == nullptr || this->parent() == nullptr )
            return;

        this->ui->taskName->setText( task->name());
        this->ui->description->setPlainText( task->description());
    }

    /**
     * @brief Description::~Description
     */
    ~Description() {
        delete this->ui;
    }

/*protected:
    void showEvent( QShowEvent *event ) override {
        this->adjustSize();
        QDialog::showEvent( event );
    }*/

private:
    Ui::Description *ui;
};
