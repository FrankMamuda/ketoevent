/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020 Armands Aleksejevs
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

/*
 * includes
 */
#include "toolbar.h"
#include <QModelIndex>

/**
 * @brief The EventToolBar class
 */
class EventToolBar final : public ToolBar {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( EventToolBar )

public:
    static EventToolBar *instance() { static EventToolBar instance; return &instance; }
    ~EventToolBar() override = default;

public slots:
    void buttonTest( const QModelIndex &index = QModelIndex());

private:
    explicit EventToolBar( QWidget *parent = nullptr );
    QAction *edit;
    QAction *remove;
};
