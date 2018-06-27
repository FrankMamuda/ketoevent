/*
 * Copyright (C) 2018 Factory #12
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

//
// includes
//

#pragma once

//
// includes
//
#include <QListView>

/**
 * @brief The TaskView class
 */
class TaskView : public QListView {
    Q_OBJECT

public:
    explicit TaskView( QWidget *parent = nullptr );
    QModelIndex check;
    QModelIndex combine;

protected:
    void mouseMoveEvent( QMouseEvent *event ) override;
};
