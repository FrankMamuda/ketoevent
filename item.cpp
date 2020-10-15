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

/*
 * includes
 */
#include "delegate.h"
#include "item.h"
#include "mainwindow.h"
#include "task.h"

/**
 * @brief Item::paint
 * @param painter
 */
void Item::paint( QPainter *painter, const QModelIndex &index ) const {
    // retrieve model values
    const bool isEditorActive = ( this->delegate->currentEditIndex() != QModelIndex() && index != this->delegate->currentEditIndex());
    const bool edit = ( this->delegate->currentEditIndex() == index );
    const bool hover = edit ? false : this->rect.contains( this->delegate->mousePos());
    const int value = this->delegate->values.isEmpty() ? 0 : this->delegate->values[index];
    const int points = Task::instance()->points( this->delegate->row( index ));
    const Task::Types type = Task::instance()->type( this->delegate->row( index ));
    const int isSelected = edit ? false : ( index == this->delegate->currentIndex());
    const bool hasValue = edit ? true : value > 0;
    const Id comboId = this->delegate->combos.isEmpty() ? Id::Invalid : this->delegate->combos[index];
    const bool isComboActive = MainWindow::instance()->isComboModeActive();
    const bool hasDescription = !Task::instance()->description( this->delegate->row( index )).isEmpty();


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
    auto drawText = [ this, painter ]( const QString &text, const QColor colour = Qt::white ) {
        QFont font( painter->font());

        // set up text
        font.setBold( true );
        font.setPointSizeF( this->rect.height() * 0.64 );
        font = Delegate::fontSizeForWidth( text, font, this->rect.width());

        // set up painter and draw text
        painter->save();
        painter->setPen( colour );
        painter->setFont( font );
        painter->setRenderHint( QPainter::TextAntialiasing, true );
        painter->drawText( this->rect.adjusted( 0, -2, 0, 0 ), text, { Qt::AlignCenter } );
        painter->restore();
    };

    // set up painter
    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->setPen( Qt::NoPen );

    // draw buttons
    switch ( this->type()) {
    case Checkable:
        if (( !isSelected && !hasValue ))
            break;

        if (( hover || isSelected ) && hasValue ) {
            drawEllipse( hover ? Red() : LtRed() );
            painter->drawPixmap( this->rect, Delegate::Remove());
        } else {
            drawEllipse(( hover || !isSelected ) ? Green() : LtGreen() );
            painter->drawPixmap( this->rect, Delegate::Check());
        }
        break;

    case Numeric:
        if ( edit )
            break;

        if ( hasValue ) {
            if ( hover || isSelected ) {
                drawEllipse( hover ? Gray() : LtGray() );
                painter->drawPixmap( this->rect, Delegate::Edit());
            } else {
                drawEllipse( Green() );
                drawText( QString::number( value ));
            }
        } else if ( !hasValue && isSelected ) {
            drawEllipse( hover ? Green() : LtGreen() );
            painter->drawPixmap( this->rect, Delegate::Number());
        }
        break;

    case Multi:
    {
        const int relativeCombo = this->delegate->relativeCombos.isEmpty() ? -1 : this->delegate->relativeCombos[comboId];

        if ( isComboActive ) {
            if ( comboId != Id::Invalid ) {
                if ( hover ) {
                    drawEllipse( hover ? Red() : LtRed() );
                    painter->drawPixmap( this->rect, Delegate::Remove());
                } else {
                    painter->setPen( { hover ? Black() : LtBlack(), 2.0 } );
                    drawEllipse( Qt::white );
                    drawText( QString::number( relativeCombo ), hover ? Black() : LtBlack());
                }
            } else {
                if ( isSelected ) {
                    drawEllipse( hover ? Blue() : LtBlue() );
                    painter->drawPixmap( this->rect, Delegate::Combine());
                }
            }
        } else {
            if ( !isSelected && type == Task::Types::Multi && !hasValue )
                break;

            if (( !hasValue || !isSelected ) && type == Task::Types::Check )
                break;

            if (( isSelected && ( hasValue || type == Task::Types::Check ))) {
                if ( comboId != Id::Invalid ) {
                    painter->setPen( { hover ? Black() : LtBlack(), 2.0 } );
                    drawEllipse( Qt::white );
                    drawText( QString::number( relativeCombo ), hover ? Black() : LtBlack());
                } else {
                    drawEllipse( hover ? Blue() : LtBlue());
                    painter->drawPixmap( this->rect, Delegate::Combine());
                }
            } else {
                drawEllipse( hasValue ? Blue() : LtBlue());
                drawText( QString::number( points ));
            }
        }
    }
        break;

    case Sum:
        if (( !isSelected && !hasValue ))
            break;

        if (( hover || isSelected ) && hasValue ) {
            drawEllipse( hover ? Red() : LtRed());
            painter->drawPixmap( this->rect, Delegate::Remove());
        } else {
            drawEllipse( !hasValue ? LtBlue() : Blue());
            drawText( QString::number( points * ( edit ? this->delegate->currentEditorValue() : value )));
        }

        break;

    case Info:
        if ( !hasDescription || !isSelected )
            break;

        drawEllipse( hover ? Yellow() : LtYellow());
        painter->drawPixmap( this->rect, Delegate::Desc());
        break;


    case Editor:
        drawEllipse( Red());
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
    const Task::Types type = Task::instance()->type( this->delegate->row( index ));
    const bool hasValue = this->delegate->values.isEmpty() ? false : this->delegate->values[index] > 0;
    const bool isComboActive = MainWindow::instance()->isComboModeActive();
    const Id comboId = this->delegate->combos.isEmpty() ? Id::Invalid : this->delegate->combos[index];

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
        if ( isComboActive )
            return comboId == Id::Invalid ? AddCombo : RemoveCombo;

        return hasValue ? Combine : NoAction;

    case Sum:
        if ( type != Task::Types::Multi )
            break;

        return hasValue ? Remove : NoAction;

    case Info:
        return InfoPopup;

    case Editor:
        break;
    }

    return NoAction;
}
