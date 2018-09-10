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

#pragma once

//
// includes
//
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include "item.h"
#include "taskview.h"

//
// classes
//
class Model;
enum class Id;
enum class Row;

/**
 * @brief The Delegate class
 */
class Delegate : public QStyledItemDelegate {
    Q_OBJECT
    friend class Item;
    friend class EditWidget;
    friend class TaskView;

public:
    explicit Delegate( QWidget *parent = nullptr ) : QStyledItemDelegate( parent ) {}
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QPoint mousePos() const { return this->m_pos; }
    QModelIndex currentIndex() const { return this->m_currentIndex; }
    QModelIndex currentEditIndex() const { return this->m_currentEditIndex; }
    TaskView *view() const { return qobject_cast<TaskView *>( this->parent()); }
    QList<Item> getItems( const QModelIndex &index ) const;
    Item::Actions action( const QModelIndex &index ) const;

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void destroyEditor( QWidget *editor, const QModelIndex &index ) const override;

    static QFont fontSizeForWidth( const QString &text, const QFont &baseFont, qreal width );
    int currentEditorValue() const;

    QModelIndex sourceIndex( const QModelIndex &index ) const;
    Row sourceRow( const QModelIndex &index ) const;

public slots:
    void setMousePos( const QPoint &pos = QPoint(), bool outside = false );
    void reset();

private:
    QPoint m_pos;
    QModelIndex m_currentIndex;
    mutable QModelIndex m_currentEditIndex;
    mutable QWidget *currentEditWidget = nullptr;

    // constants
    static const int ButtonWidth = 32;
    static const int SmallWidth = 16;
    static const int ItemHeight = 32;

    // pixmaps
    static const QPixmap Check()  { static QPixmap p( ":/icons/check" ); return p; }
    static const QPixmap Edit()   { static QPixmap p( ":/icons/editLog" ); return p; }
    static const QPixmap Remove() { static QPixmap p( ":/icons/removeLog" ); return p; }
    static const QPixmap Number() { static QPixmap p( ":/icons/number" ); return p; }
    static const QPixmap Combine() { static QPixmap p( ":/icons/combine" ); return p; }
    static const QPixmap Cross()  { static QPixmap p( ":/icons/cross" ); return p; }
    static const QPixmap Equals() { static QPixmap p( ":/icons/equals" ); return p; }

    // button sizes
    mutable QMap<QModelIndex, QRect> rectSizes;
    mutable QMap<QModelIndex, int> buttonSizes;
    mutable QMap<QModelIndex, int> values;
    mutable QMap<QModelIndex, Id> combos;
    mutable QMap<Id, int> relativeCombos;
    mutable int lastRelativeCombo = 0;
    mutable int m_value = 0;
};

/**
 * @brief The EditWidget class
 */
class EditWidget : public QSpinBox {
    Q_OBJECT

public:
    explicit EditWidget( const Delegate *d, const QModelIndex &i, QWidget *parent = nullptr ) : QSpinBox( parent ), delegate( d ), index( i ) {}

protected:
    void paintEvent( QPaintEvent *event ) override;

private:
    const Delegate *delegate;
    QModelIndex index;
};
