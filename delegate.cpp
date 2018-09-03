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
#include <QSpinBox>
#include <QDebug>
#include <QStylePainter>
#include "task.h"

/**
 * @brief Delegate::paint
 * @param painter
 * @param option
 * @param index
 */
void Delegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    const Task::Types type = Task::instance()->type( index.row());
    const int buttonSize = ( type == Task::Types::Multi ) ? Delegate::ButtonWidth * 3 + Delegate::SmallWidth * 2 : Delegate::ButtonWidth * 2 + Delegate::SmallWidth;
    const QRect rect( option.rect.left(), option.rect.top(), option.rect.width() - buttonSize, Delegate::ItemHeight );
    const bool edit = this->currentEditIndex() == index;

    // store rectSize
    this->rectSizes[index] = rect;

    // draw cross/equals lambda
    auto drawCrossEquals = [ this, painter, index, rect, type, edit ]() {
        const int isSelected = edit ? false : ( index == this->currentIndex());
#ifdef VALUE_CACHE
        const bool hasValue = this->values[index] > 0;
#else
        const bool hasValue = Task::instance()->multiplier( index.row()) > 0;
#endif
        QRect small( rect.right() + Delegate::ButtonWidth, rect.top(), Delegate::SmallWidth, Delegate::ItemHeight );

        if ( type == Task::Types::Check )
            return;

        if (( hasValue && !isSelected ) || ( !hasValue && isSelected )) {
            painter->drawPixmap( small, Cross());
            painter->drawPixmap( small.translated( Delegate::ButtonWidth + Delegate::SmallWidth, 0 ), Equals());
        }
    };

    // draw items
    if ( this->currentEditIndex() == QModelIndex() || edit ) {
        if ( edit ) {
            QColor highlight( option.palette.highlight().color());
            highlight.setAlpha( 128 );
            painter->fillRect( option.rect, highlight );
        }

        foreach ( const Item &item, this->getItems( index ))
            item.paint( painter, index );

        drawCrossEquals();
    }

    // set up font and draw task name
    painter->setFont( { option.font.family(), static_cast<int>( Delegate::ItemHeight * 0.4 ) } );
    painter->drawText( rect, QFontMetrics( painter->font()).elidedText( Task::instance()->name( index.row()), Qt::ElideRight, rect.width()), { Qt::AlignLeft | Qt::AlignVCenter } );

    // disable view
    if ( this->currentEditIndex() != QModelIndex() && !edit ) {
        QColor foreground( option.palette.foreground().color());
        foreground.setAlpha( 16 );
        painter->fillRect( option.rect, foreground );
    }
}

/**
 * @brief Delegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize Delegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QSize size( QStyledItemDelegate::sizeHint( option, index ));

    // TODO: proper width
    //size.setWidth( this->view()->viewport()->width() );
    size.setHeight( Delegate::ItemHeight );

#ifdef VALUE_CACHE
    this->values[index] = Task::instance()->multiplier( index.row());
#endif

    return size;
}

/**
 * @brief Delegate::getItems
 * @return
 */
QList<Item> Delegate::getItems( const QModelIndex &index ) const {
    const QRect rect( this->rectSizes[index] );
    QRect button( rect.right(), rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight );

    return Task::instance()->type( index.row()) == Task::Types::Multi ?
                QList<Item>() << Item( Item::Multi, button, this ) <<
                                         Item( Item::Numeric, button.translated( Delegate::ButtonWidth + Delegate::SmallWidth, 0 ), this ) <<
                                         Item( Item::Sum, button.translated(( Delegate::ButtonWidth + Delegate::SmallWidth ) * 2, 0 ), this )
                                         :
                                         QList<Item>() <<
                                         Item( Item::Multi, button, this ) <<
                                         Item( Item::Checkable, button.translated( Delegate::ButtonWidth + Delegate::SmallWidth, 0 ), this );
}

/**
 * @brief Delegate::action
 * @param index
 * @return
 */
Item::Actions Delegate::action( const QModelIndex &index ) const {
    Item::Actions action = Item::NoAction;

    foreach ( const Item &item, this->getItems( index )) {
        action = item.action( index );

        if ( action != Item::NoAction )
            return action;
    }

    return action;
}

/**
 * @brief Delegate::setMousePos
 * @param pos
 */
void Delegate::setMousePos( const QPoint &pos, bool outside ) {
    int y;

    // failsafe
    if ( this->view() == nullptr || this->m_pos == pos )
        return;

    // store position
    this->m_pos = pos;

    // leave event
    if ( outside ) {
        this->view()->update( this->currentIndex());
        this->m_currentIndex = QModelIndex();
    }

    // search for a rect that contains mouse
    bool found = false;
    for ( y = 0; y < Task::instance()->count(); y++ ) {
        const QModelIndex index( this->view()->model()->index( y, Task::instance()->Name ));

        if ( this->view()->visualRect( index ).contains( pos )) {
            this->view()->update( this->currentIndex());
            this->m_currentIndex = index;
            this->view()->update( this->currentIndex());

            found = true;
            break;
        }
    }

    if ( !found ) {
        this->view()->update( this->currentIndex());
        this->m_currentIndex = QModelIndex();
    }
}

/**
 * @brief Delegate::createEditor
 * @param parent
 * @return
 */
QWidget *Delegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index ) const {
    EditWidget *edit( new EditWidget( this, index, parent ));

    // store current index
    this->m_currentEditIndex = index;

    // set up widget
    // TODO:
    edit->setMaximum( 4096 );
    edit->setAlignment( Qt::AlignCenter );
    edit->setButtonSymbols( QAbstractSpinBox::NoButtons );
    edit->setStyleSheet( "QSpinBox { background-color: transparent; color: white; text-align: center; selection-background-color: transparent; } QSpinBox::up-button { width: 0px; } QSpinBox::down-button { width: 0px; }" );

    // return a newly constructed editor widget
    return edit;
}

/**
 * @brief Delegate::setEditorData
 * @param editor
 * @param index
 */
void Delegate::setEditorData( QWidget *editor, const QModelIndex &index ) const {
    EditWidget *editWidget( qobject_cast<EditWidget*>( editor ));

#ifdef VALUE_CACHE
    const int value = this->values[index];
#else
    const int value = Task::instance()->multiplier( index.row());
#endif

    // TODO: ALSO SET LIMITS
    editWidget->setValue( value );
    this->m_value = value;
}

/**
 * @brief Delegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void Delegate::setModelData( QWidget *editor, QAbstractItemModel *, const QModelIndex &index ) const {
    EditWidget *editWidget( qobject_cast<EditWidget*>( editor ));
    editWidget->interpretText();
    Task::instance()->setMultiplier( index.row(), editWidget->value() );
}

/**
 * @brief Delegate::updateEditorGeometry
 * @param editor
 * @param option
 */
void Delegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    editor->setGeometry( { this->rectSizes[index].right() + Delegate::ButtonWidth + Delegate::SmallWidth, option.rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight } );
}

/**
 * @brief Delegate::destroyEditor
 * @param editor
 * @param index
 */
void Delegate::destroyEditor( QWidget *editor, const QModelIndex &index ) const {
    // reset index and delete widget
    this->m_currentEditIndex = QModelIndex();
    QStyledItemDelegate::destroyEditor( editor, index );
}

/**
 * @brief EditWidget::paintEvent
 * @param event
 */
void EditWidget::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );
    const Item item( Item::Editor, QRect( 0, 0, Delegate::ButtonWidth, Delegate::ItemHeight ), this->delegate );

    // set font size determined in fontSizeForWidth
    this->setFont( this->font );

    // paint background
    item.paint( &painter, this->index );

    // paint input
    QSpinBox::paintEvent( event );
}

/**
 * @brief Delegate::fontSizeForWidth
 * @param text
 * @param baseFont
 * @param width
 * @return
 */
QFont Delegate::fontSizeForWidth( const QString &text, const QFont &baseFont, qreal width ) {
    QFont font( baseFont );

    // ignore invalid sizes
    if ( width <= 4.0 )
        return font;

    // determine optimal font size
    forever {
        const QFontMetricsF fm( font );

        if ( font.pointSize() < 4 )
            break;

        if ( fm.width( text ) < static_cast<qreal>( width ) * 0.90 )
            break;

        font.setPointSizeF( font.pointSizeF() * 0.75 );
    }

    return font;
}

/**
 * @brief EditWidget::validate
 * @param input
 * @param pos
 * @return
 */
QValidator::State EditWidget::validate( QString &input, int &pos ) const {
    bool ok;

    // get font
    QFont font( QSpinBox::font());
    font.setBold( true );
    font.setPointSizeF( this->height() * 0.64 );

    // store font & value
    this->font = Delegate::fontSizeForWidth( input, font, this->width());
    this->delegate->m_value = input.toInt( &ok );
    if ( !ok )
        this->delegate->m_value = 0;

    // update item on change
    this->delegate->view()->update( this->delegate->currentEditIndex());

    // continue validation
    return QSpinBox::validate( input, pos );
}
