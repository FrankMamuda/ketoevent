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
#include <QListView>
#include <QResizeEvent>

/**
 * @brief The TaskView class
 */
class TaskView : public QListView {
    Q_OBJECT
    friend class Delegate;

public:
    /**
     * @brief ListView
     * @param parent
     */
    explicit TaskView( QWidget *parent = nullptr ) : QListView( parent ) { this->setMouseTracking( true ); }
    ~TaskView() override = default;

protected:
    /**
     * @brief resizeEvent
     * @param event
     */
    void resizeEvent( QResizeEvent *event ) override { QListView::resizeEvent( event ); }
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void leaveEvent( QEvent *event ) override;

private:
    QModelIndex m_focus;
};
