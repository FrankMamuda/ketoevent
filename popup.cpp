/*
 * Copyright (C) 2013-2019 Factory #12
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
#include "popup.h"
#include <QPainter>
#include <QDebug>
#include <QPalette>
#include <QMouseEvent>
#include <QTimer>

/**
 * @brief Popup::Popup3
 * @param parent
 */
Popup::Popup( QWidget *parent, const QString &text, const int timeout ) : QDialog( parent ),
    shadow( new QGraphicsDropShadowEffect( this )),
    layout( new QHBoxLayout()),
    label( new QLabel( this )) {

    // setup window
    this->setAutoFillBackground( false );
    this->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint );
    this->setAttribute( Qt::WA_TranslucentBackground, true );
    //this->setAttribute( Qt::WA_DeleteOnClose, true );
    this->setWindowModality( Qt::ApplicationModal );
    this->setModal( true );

    // setup shadow
    this->shadow->setBlurRadius( this->shadowBlurRadius );
    this->shadow->setOffset( this->shadowOffset );
    this->setGraphicsEffect( shadow );

    // setup label
    const int o0 = this->shadowBlurRadius - this->shadowOffset;
    const int o1 = this->shadowBlurRadius + this->shadowOffset;
    this->setText( text );
    this->label->setStyleSheet( QString( "QLabel {"
                          "padding-top: %1px;"
                          "padding-left: %2px;"
                          "padding-right: %3px;"
                          "padding-bottom: %3px; }"
                          )
                          .arg( 20 + o0 )
                          .arg( o0 )
                          .arg( o1 ));
    this->setTextAlignment( Qt::AlignCenter );
    this->label->setWordWrap( true );

    // setup layout
    layout->addWidget( label );
    this->setLayout( layout );

    // setup popup shape
    this->setupShape();

    // setup options
    this->setTimeOut( timeout );
    this->pointAt( QCursor::pos());
}

/**
 * @brief Popup3::setupShape
 */
void Popup::setupShape() {
    const int o0 = this->shadowBlurRadius - this->shadowOffset;
    const int o1 = this->shadowBlurRadius + this->shadowOffset;
    const int padding = 16;
    const int w = this->width();
    const int h = this->height();

    // setup polygon
    this->arrowHead = QPoint( o0 + 40,  0  + o0 );
    this->poly = QPolygon( QVector<QPoint>() <<
                           QPoint( o0,       20 + o0 ) << // UL
                           QPoint( o0 + 20,  20 + o0 ) << // arrow base L
                           arrowHead <<                   // arrow head
                           QPoint( o0 + 60,  20 + o0 ) << // arrow base R
                           QPoint( w - o1,   20 + o0 ) << // UR
                           QPoint( w - o1,   h - o1 ) <<  // LR
                           QPoint( o0,       h - o1 ));   // LL

    // set minimum size to avoid uglies
    this->setMinimumSize( o0 + 60 + o1 + padding, o0 + 20 + o1 + padding );
}

/**
 * @brief Popup::setText
 * @param text
 */
void Popup::setText( const QString &text ) {
    this->label->setText( text );
}

/**
 * @brief Popup::pointAt
 * @param point
 */
void Popup::pointAt( const QPoint &point ) {
    const QPoint pos( point.x() - this->arrowHead.x(), point.y() - this->arrowHead.y());
    this->move( pos );
}

/**
 * @brief Popup::setTimeOut
 * @param msec
 */
void Popup::setTimeOut( const int msec ) {
    if ( msec < 100 || msec > 60000 )
        return;

    // close when timer runs out
    QTimer::singleShot( msec, this, SLOT( close()));
}

/**
 * @brief Popup::paintEvent
 * @param event
 */
void Popup::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );

    // draw popup shape
    painter.setPen( Qt::transparent );
    painter.setBrush( this->palette().color( QPalette::Background ));
    painter.drawPolygon( this->poly, Qt::OddEvenFill );

    // draw label
    QDialog::paintEvent( event );
}

/**
 * @brief Popup::mousePressEvent
 * @param event
 */
void Popup::mousePressEvent( QMouseEvent *event ) {
    event->accept();
}

/**
 * @brief Popup::mouseReleaseEvent
 * @param event
 */
void Popup::mouseReleaseEvent( QMouseEvent *event ) {
    event->accept();

    if ( !this->poly.containsPoint( event->pos(), Qt::OddEvenFill ))
        this->close();
}
