/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020-2024 Armands Aleksejevs
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

/*
 * includes
 */
#include "popup.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTimer>

/**
 * @brief Popup::Popup
 * @param parent
 */
Popup::Popup(QWidget *parent, const QString &text, const int timeout)
    : QDialog(parent)
    , shadow(new QGraphicsDropShadowEffect(this))
    , layout(new QHBoxLayout())
    , label(new QLabel(this)) {

    // setup window
    setAutoFillBackground(false);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    // setAttribute( Qt::WA_DeleteOnClose, true );
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    // setup shadow
    shadow->setBlurRadius(shadowBlurRadius);
    shadow->setOffset(shadowOffset);
    setGraphicsEffect(shadow);

    // setup label
    const int o0 = shadowBlurRadius - shadowOffset;
    const int o1 = shadowBlurRadius + shadowOffset;
    setText(text);
    label->setStyleSheet(QString("QLabel {"
                                       "padding-top: %1px;"
                                       "padding-left: %2px;"
                                       "padding-right: %3px;"
                                       "padding-bottom: %3px; }")
                                   .arg(20 + o0)
                                   .arg(o0)
                                   .arg(o1));
    setTextAlignment(Qt::AlignCenter);
    label->setWordWrap(true);

    // setup layout
    layout->addWidget(label);
    setLayout(layout);

    // setup popup shape
    setupShape();

    // setup options
    setTimeOut(timeout);
    pointAt(QCursor::pos());
}

/**
 * @brief Popup::setupShape
 */
void Popup::setupShape() {
    const int o0 = shadowBlurRadius - shadowOffset;
    const int o1 = shadowBlurRadius + shadowOffset;
    const int padding = 16;
    const int w = width();
    const int h = height();

    // setup polygon
    // NOTE: a popup of a maxmimzed window will most likely be drawn off screen, so reverse the arrow location
    // NOTE: not implemented properly yet (disabled)
    const bool inverse = false; // parentWidget() == nullptr ? false : parentWidget()->isMaximized();

    arrowHead = inverse ? QPoint(w - 40 - o1, 0 + o0) : QPoint(o0 + 40, 0 + o0);
    poly = QPolygon(QVector<QPoint>() << QPoint(o0, 20 + o0) << // UL
        QPoint(inverse ? w - 60 - o1 : o0 + 20, 20 + o0) << // arrow base L
        arrowHead << // arrow head
        QPoint(inverse ? w - 20 - o1 : o0 + 60, 20 + o0) << // arrow base R
        QPoint(w - o1, 20 + o0) << // UR
        QPoint(w - o1, h - o1) << // LR
        QPoint(o0, h - o1)); // LL

    // set minimum size to avoid uglies
    setMinimumSize(o0 + 60 + o1 + padding, o0 + 20 + o1 + padding);
}

/**
 * @brief Popup::setText
 * @param text
 */
void Popup::setText(const QString &text) { label->setText(text); }

/**
 * @brief Popup::pointAt
 * @param point
 */
void Popup::pointAt(const QPoint &point) {
    // NOTE: not implemented properly yet (disabled)
    const bool inverse = false; // = parentWidget() == nullptr ? false : parentWidget()->isMaximized();
    Q_UNUSED(inverse)

    const QPoint pos(point.x() - arrowHead.x(), point.y() - arrowHead.y());
    move(pos);
}

/**
 * @brief Popup::setTimeOut
 * @param msec
 */
void Popup::setTimeOut(const int msec) {
    if (msec < 100 || msec > 60000) return;

    // close when timer runs out
    QTimer::singleShot(msec, this, SLOT(close()));
}

/**
 * @brief Popup::paintEvent
 * @param event
 */
void Popup::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // draw popup shape
    painter.setPen(Qt::transparent);
    painter.setBrush(palette().color(QPalette::Window));
    painter.drawPolygon(poly, Qt::OddEvenFill);

    // draw label
    QDialog::paintEvent(event);
}

/**
 * @brief Popup::mousePressEvent
 * @param event
 */
void Popup::mousePressEvent(QMouseEvent *event) { event->accept(); }

/**
 * @brief Popup::mouseReleaseEvent
 * @param event
 */
void Popup::mouseReleaseEvent(QMouseEvent *event) {
    event->accept();

    if (!poly.containsPoint(event->pos(), Qt::OddEvenFill)) close();
}
