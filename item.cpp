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
#include "delegate.h"
#include "item.h"
#include "task.h"

// initialize colours
const QColor Item::Green   = { 57, 174, 57, 196 };
const QColor Item::LtGreen = lighter( Green );
const QColor Item::Blue    = { 0, 174, 255, 196 };
const QColor Item::LtBlue  = lighter( Blue );
const QColor Item::Gray    = { 64, 64, 64, 196 };
const QColor Item::LtGray  = lighter( Gray );
const QColor Item::Red     = { 190, 0, 0, 196 };
const QColor Item::LtRed   = lighter( Red );

/**
 * @brief Item::paint
 * @param painter
 */
void Item::paint( QPainter *painter, const QModelIndex &index ) const {
    // retrieve model values
    const bool isEditorActive = ( this->delegate->currentEditIndex() != QModelIndex() && index != this->delegate->currentEditIndex());
    const bool edit = ( this->delegate->currentEditIndex() == index );
    const bool hover = edit ? false : this->rect.contains( this->delegate->mousePos());
    const int value = Task::instance()->logValue( index.row());
    const int multi = Task::instance()->multi( index.row());
    const Task::Types type = Task::instance()->type( index.row());
    const int isSelected = edit ? false : ( index == this->delegate->currentIndex());
    const bool hasValue = edit ? true : value > 0;

    // don't draw anything in edit mode
    if ( isEditorActive && !edit )
        return;

    // ellipse drawing lambda
    auto drawEllipse = [ this, painter ]( const QColor &colour ) {
        const qreal penWidth = painter->pen().widthF();
        painter->setBrush( colour );
        painter->drawEllipse( penWidth > 0.0 ? QRectF( this->rect.x() + penWidth / 2.0, this->rect.y() + penWidth / 2.0, this->rect.width() - penWidth, this->rect.height() - penWidth ) : this->rect );
    };

    // text drawing lambda
    // NOTE: there is not that much of a performance penalty to do caching
    auto drawText = [ this, painter ]( const QString &text ) {
        QFont font( painter->font());

        // set up text
        font.setBold( true );
        font.setPointSizeF( this->rect.height() * 0.64 );
        font = Delegate::fontSizeForWidth( text, font, this->rect.width());

        // set up painter and draw text
        painter->save();
        painter->setPen( Qt::white );
        painter->setFont( font );
        painter->setRenderHint( QPainter::TextAntialiasing, true );
        painter->drawText( this->rect, text, { Qt::AlignCenter } );
        painter->restore();
    };

    // set up painter
    painter->save();
    painter->setRenderHint( QPainter::HighQualityAntialiasing, true );
    painter->setPen( Qt::NoPen );

    // draw buttons
    switch ( this->type()) {
    case Checkable:
        if (( !isSelected && !hasValue ))
            break;

        if (( hover || isSelected ) && hasValue ) {
            drawEllipse( hover ? Red : LtRed );
            painter->drawPixmap( this->rect, Delegate::Remove());
        } else {
            drawEllipse(( hover || !isSelected ) ? Green : LtGreen );
            painter->drawPixmap( this->rect, Delegate::Check());
        }
        break;

    case Numeric:
        if ( edit )
            break;

        if ( hasValue ) {
            if ( hover || isSelected ) {
                drawEllipse( hover ? Gray : LtGray );
                painter->drawPixmap( this->rect, Delegate::Edit());
            } else {
                drawEllipse( Green );
                drawText( QString::number( value ));
            }
        } else if ( !hasValue && isSelected ) {
            drawEllipse( hover ? Green : LtGreen );
            painter->drawPixmap( this->rect, Delegate::Number());
        }
        break;

    case Multi:
        if ( !isSelected && type == Task::Types::Multi && !hasValue )
            break;

        if (( !hasValue || !isSelected ) && type == Task::Types::Check )
            break;

        if ( isSelected && ( hasValue || type == Task::Types::Check )) {
            drawEllipse( hover ? Blue : LtBlue );
            painter->drawPixmap( this->rect, Delegate::Combine());
        } else {
            drawEllipse( hasValue ? Blue : LtBlue );
            drawText( QString::number( multi ));
        }
        break;

    case Sum:
        if (( !isSelected && !hasValue ))
            break;

        if (( hover || isSelected ) && hasValue ) {
            drawEllipse( hover ? Red : LtRed );
            painter->drawPixmap( this->rect, Delegate::Remove());
        } else {
            drawEllipse( !hasValue ? LtBlue : Blue );
            drawText( QString::number( multi * ( edit ? this->delegate->currentEditorValue() : value )));
        }

        break;

    case Editor:
        drawEllipse( Red );
        break;
    }

    // restore previous state
    painter->restore();
}

/**
 * @brief Item::action
 * @param index
 * @return
 */
Item::Actions Item::action( const QModelIndex &index ) const {
    // retrieve model values
    const bool edit = ( this->delegate->currentEditIndex() != QModelIndex() && index != this->delegate->currentEditIndex());
    const Task::Types type = Task::instance()->type( index.row());
    const bool hasValue = Task::instance()->logValue( index.row()) > 0;

    // don't allow clicks in editing mode
    if ( edit )
        return NoAction;

    if ( !this->rect.contains( this->delegate->mousePos()) || index != this->delegate->currentIndex())
        return NoAction;

    // test buttons
    switch ( this->type()) {
    case Checkable:
        return hasValue ? Remove : Set;

    case Numeric:
        if ( type != Task::Types::Multi )
            break;

        return hasValue ? Edit : SetNumeric;

    case Multi:
        return hasValue ? Combine : NoAction;

    case Sum:
        if ( type != Task::Types::Multi )
            break;

        return hasValue ? Remove : NoAction;

    case Editor:
        break;
    }

    return NoAction;
}
