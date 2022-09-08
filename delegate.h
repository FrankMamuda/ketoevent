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
    Q_DISABLE_COPY_MOVE( Delegate )
    friend class Item;
    friend class EditWidget;
    friend class TaskView;
    friend class MainWindow;

public:
    explicit Delegate( QWidget *parent = nullptr ) : QStyledItemDelegate( parent ) {}
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    [[nodiscard]] QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    [[nodiscard]] QPoint mousePos() const { return this->m_pos; }
    [[nodiscard]] QModelIndex currentIndex() const { return this->m_currentIndex; }
    [[nodiscard]] QModelIndex currentEditIndex() const { return this->m_currentEditIndex; }
    [[nodiscard]] TaskView *view() const { return qobject_cast<TaskView *>( this->parent()); }
    [[nodiscard]] QList<Item> getItems( const QModelIndex &index ) const;
    [[nodiscard]] Item::Actions action( const QModelIndex &index ) const;

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void destroyEditor( QWidget *editor, const QModelIndex &index ) const override;

    static QFont fontSizeForWidth( const QString &text, const QFont &baseFont, qreal width );
    [[nodiscard]] int currentEditorValue() const;

    [[nodiscard]] Row row( const QModelIndex &index ) const;

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
    static const QPixmap Check()  { static QPixmap p( QIcon::fromTheme( "check" ).pixmap( 32, 32 )); return p; }
    static const QPixmap Edit()   { static QPixmap p( QIcon::fromTheme( "edit_log" ).pixmap( 32, 32 )); return p; }
    static const QPixmap Remove() { static QPixmap p( QIcon::fromTheme( "remove_log" ).pixmap( 32, 32 )); return p; }
    static const QPixmap Number() { static QPixmap p( QIcon::fromTheme( "number" ).pixmap( 32, 32 )); return p; }
    static const QPixmap Combine() { static QPixmap p( QIcon::fromTheme( "combine" ).pixmap( 32, 32 )); return p; }
    static const QPixmap Cross()  { static QPixmap p( QIcon::fromTheme( "cross" ).pixmap( 16, 32 )); return p; }
    static const QPixmap Equals() { static QPixmap p( QIcon::fromTheme( "equals" ).pixmap( 16, 32 )); return p; }
    static const QPixmap Desc()   { static QPixmap p( QIcon::fromTheme( "description" ).pixmap( 32, 32 )); return p; }

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
    explicit EditWidget( const Delegate *d, const QModelIndex &i, QWidget *parent = nullptr ) : QSpinBox( parent ), delegate( d ), index( i ) { this->setMinimum( -999 ); }

protected:
    void paintEvent( QPaintEvent *event ) override;

private:
    const Delegate *delegate;
    QModelIndex index;
};
