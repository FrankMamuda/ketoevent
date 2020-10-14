/*
 * Copyright (C) 2013-2019 Factory #12
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
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>

/**
 * @brief The Popup class
 */
class Popup : public QDialog {
    Q_OBJECT

public:
    explicit Popup( QWidget *widget = nullptr, const QString &text = QString(), const int timeout = 5000 );
    ~Popup() override { delete this->shadow; delete this->layout; }

public slots:
    void setText( const QString &text );
    void setTextAlignment( const Qt::Alignment &alignment ) { this->label->setAlignment( alignment ); }
    void pointAt( const QPoint &point );
    void setTimeOut( const int msec = 5000 );

private slots:
    void setupShape();

protected:
    void resizeEvent( QResizeEvent *event ) override { this->setupShape(); QDialog::resizeEvent( event ); }
    void paintEvent( QPaintEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

private:
    const int shadowBlurRadius = 16;
    const int shadowOffset = 2;

    QPolygon poly;
    QGraphicsDropShadowEffect *shadow;
    QHBoxLayout *layout;
    QLabel *label;
    QPoint arrowHead;
};
