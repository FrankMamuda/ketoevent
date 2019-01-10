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
#include "mainwindow.h"
#include "task.h"

// select * from logs where logs.value>0 and ( comboId=1 or comboId=-1)

/**
 * @brief Delegate::paint
 * @param painter
 * @param option
 * @param index
 */
void Delegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    const Task::Types type = Task::instance()->type( this->row( index ));
    const QFont font = Task::instance()->data( index, Qt::FontRole ).value<QFont>();
    const int buttonSize = this->buttonSizes.isEmpty() ? 0 : this->buttonSizes[index];
    const QRect rect( option.rect.left(), option.rect.top(), option.rect.width() - buttonSize, Delegate::ItemHeight );
    const bool edit = this->currentEditIndex() == index;
    const Id comboId = this->combos.isEmpty() ? Id::Invalid : this->combos[index];
    const bool isComboActive = MainWindow::instance()->isComboModeActive();
    const bool isEventActive = MainWindow::instance()->currentEvent() != Row::Invalid;
    const bool isTeamActive = MainWindow::instance()->currentTeam() != Row::Invalid;

    if ( !isTeamActive )
        return;

    // save state
    painter->save();
    painter->setRenderHint( QPainter::HighQualityAntialiasing, true );
    painter->setRenderHint( QPainter::TextAntialiasing, true );

    // combo
    if ( comboId != Id::Invalid ) {
        if ( !this->relativeCombos.contains( comboId ))
            this->relativeCombos[comboId] = ++this->lastRelativeCombo;

    }

    // abort if no active combos are visible
    if ( isComboActive && !this->combos.values().contains( MainWindow::instance()->currentComboId()))
        MainWindow::instance()->setTaskFilter();

    // store rectSize
    this->rectSizes[index] = rect;

    // draw cross/equals lambda
    auto drawCrossEquals = [ this, painter, index, rect, type, edit, isComboActive, isEventActive, isTeamActive ]() {
        const int isSelected = edit ? false : ( index == this->currentIndex());
        const bool hasValue = this->values.isEmpty() ? false : this->values[index] > 0;
        QRect small( rect.right() + Delegate::ButtonWidth, rect.top(), Delegate::SmallWidth, Delegate::ItemHeight );

        if ( type == Task::Types::Check || isComboActive || !isEventActive || !isTeamActive )
            return;

        if (( hasValue && !isSelected ) || ( !hasValue && isSelected )) {
            painter->drawPixmap( small, Cross());
            painter->drawPixmap( small.translated( Delegate::ButtonWidth + Delegate::SmallWidth, 0 ), Equals());
        }
    };

    // draw items
    if (( this->currentEditIndex() == QModelIndex() || edit || isComboActive ) && isEventActive && isTeamActive ) {
        if ( edit || ( isComboActive && comboId != Id::Invalid )) {
            QColor highlight( option.palette.highlight().color());
            highlight.setAlpha( 128 );
            painter->fillRect( option.rect, highlight );
        }

        foreach ( const Item &item, this->getItems( index ))
            item.paint( painter, index );

        drawCrossEquals();
    }

    // set up font and draw task name
    painter->setFont( { option.font.family(), static_cast<int>( Delegate::ItemHeight * 0.4 ), font.weight(), font.italic() } );
    painter->drawText( rect, QFontMetrics( painter->font()).elidedText( Task::instance()->name( this->row( index )), Qt::ElideRight, rect.width()), { Qt::AlignLeft | Qt::AlignVCenter } );

    // disable view
    if ( this->currentEditIndex() != QModelIndex() && !edit && !isComboActive ) {
        QColor foreground( option.palette.foreground().color());
        foreground.setAlpha( 16 );
        painter->fillRect( option.rect, foreground );
    }

    // restore state
    painter->restore();
}

/**
 * @brief Delegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize Delegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QSize size( QStyledItemDelegate::sizeHint( option, index ));
    const Task::Types type = Task::instance()->type( this->row( index ));
    const int buttonSize = MainWindow::instance()->isComboModeActive() ? Delegate::ButtonWidth : (( type == Task::Types::Multi ) ? Delegate::ButtonWidth * 3 + Delegate::SmallWidth * 2 : Delegate::ButtonWidth * 2 + Delegate::SmallWidth );
    const bool isTeamActive = MainWindow::instance()->currentTeam() != Row::Invalid;

    this->buttonSizes[index] = buttonSize;
    size.setWidth( buttonSize );
    size.setHeight( Delegate::ItemHeight );

    if ( isTeamActive ) {
        this->combos[index] = Task::instance()->comboId( this->row( index ));
        this->values[index] = Task::instance()->multiplier( this->row( index ));
    }

    return size;
}

/**
 * @brief Delegate::getItems
 * @return
 */
QList<Item> Delegate::getItems( const QModelIndex &index ) const {
    const QRect rect( this->rectSizes.isEmpty() ? QRect() : this->rectSizes[index] );
    QRect button( rect.right(), rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight );
    const Item multi( Item::Multi, button, this );

    if ( MainWindow::instance()->isComboModeActive())
        return QList<Item>() << multi;

    return Task::instance()->type( this->row( index )) == Task::Types::Multi ?
                QList<Item>() << multi <<
                                 Item( Item::Numeric, button.translated( Delegate::ButtonWidth + Delegate::SmallWidth, 0 ), this ) <<
                                 Item( Item::Sum, button.translated(( Delegate::ButtonWidth + Delegate::SmallWidth ) * 2, 0 ), this )
                                 :
                                 QList<Item>() <<
                                 multi <<
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
        if ( this->currentIndex().isValid() && this->view() != nullptr )
            this->view()->update( this->currentIndex());

        this->m_currentIndex = QModelIndex();
    }
}

/**
 * @brief Delegate::reset
 */
void Delegate::reset() {
    this->m_pos = QPoint();
    this->m_currentIndex = QModelIndex();
    this->m_currentEditIndex = QModelIndex();
    this->currentEditWidget = nullptr;

    // button sizes
    this->rectSizes.clear();
    this->buttonSizes.clear();
    this->values.clear();
    this->combos.clear();
    this->relativeCombos.clear();
    this->lastRelativeCombo = 0;
    this->m_value = 0;
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
    this->currentEditWidget = edit;

    // set up widget
    edit->setMaximum( Task::instance()->multi( this->row( index )));
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
    const int value = this->values.isEmpty() ? 0 : this->values[index];
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
    Task::instance()->setMultiplier( this->row( index ), editWidget->value() );
}

/**
 * @brief Delegate::updateEditorGeometry
 * @param editor
 * @param option
 */
void Delegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    editor->setGeometry( { this->rectSizes.isEmpty() ? 0 : this->rectSizes[index].right() + Delegate::ButtonWidth + Delegate::SmallWidth, option.rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight } );
}

/**
 * @brief Delegate::destroyEditor
 * @param editor
 * @param index
 */
void Delegate::destroyEditor( QWidget *editor, const QModelIndex &index ) const {
    // reset index and delete widget
    this->m_currentEditIndex = QModelIndex();
    this->currentEditWidget = nullptr;
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
    //this->setFont( this->font );

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
 * @brief Delegate::currentEditorValue
 * @return
 */
int Delegate::currentEditorValue() const {
    if ( this->currentEditIndex() == QModelIndex() || this->currentEditWidget == nullptr )
        return 0;

    // get widget
    EditWidget *edit( qobject_cast<EditWidget*>( this->currentEditWidget ));

    // get font
    QFont font( edit->font());
    font.setBold( true );
    font.setPointSizeF( edit->height() * 0.64 );
    edit->setFont( Delegate::fontSizeForWidth( QString::number( edit->value()), font, edit->width()));

    // update item on change
    this->view()->update( this->currentEditIndex());

    // return value
    return edit->value();
}

/**
 * @brief Delegate::proxyRow
 * @param index
 * @return
 */
Row Delegate::row( const QModelIndex &index ) const {
    const Table *table( qobject_cast<const Table*>( index.model()));

    if ( table == nullptr ) {
        qDebug() << this->tr( "invalid model" ) << index.model()->objectName();
        return Row::Invalid;
    }

    return table->row( index );
}
