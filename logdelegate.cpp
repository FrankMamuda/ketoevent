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
#include <QSpinBox>
#include <QDebug>
#include <QPainter>
#include "logdelegate.h"
#include "log.h"
#include "task.h"
#include "logeditor.h"
#include "taskview.h"
#include "mainwindow.h"

/**
 * @brief LogDelegate::LogDelegate
 * @param parent
 */
LogDelegate::LogDelegate( QObject *parent ) : QStyledItemDelegate( parent ), m_edit( false ) { }

/**
 * @brief LogDelegate::createEditor
 * @param parent
 * @return
 */
QWidget *LogDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index ) const {
    LogEditor *editor;
    int row;

    qDebug() << "create editor";

    row = index.model()->data( index, Log::TaskRole ).toInt();
    editor = new LogEditor( parent );
    //editor->setFrame(false);
    if ( row != -1 )
        editor->setName( Task::instance()->name( row ));

    this->m_edit = true;
    return editor;//new QSpinBox( parent );//editor;
}

/**
 * @brief LogDelegate::setEditorData
 * @param editor
 * @param index
 */
void LogDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const {
    Q_UNUSED( editor )
    Q_UNUSED( index )

    /*QSpinBox *spinBox;
    int value;

    value = index.model()->data( index, Log::MultiRole ).toInt();
    spinBox = qobject_cast<QSpinBox*>( editor );
    spinBox->setValue( value );*/
}

/**
 * @brief LogDelegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void LogDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const {
    Q_UNUSED( editor )
    Q_UNUSED( model )
    Q_UNUSED( index )

    /*QSpinBox *spinBox;
    int value;

    spinBox = qobject_cast<QSpinBox*>( editor );
    spinBox->interpretText();
    value = spinBox->value();
    model->setData( index, value, Log::MultiRole );*/
    qDebug() << "end edit";

    this->m_edit = false;
}

/**
 * @brief LogDelegate::updateEditorGeometry
 * @param editor
 * @param option
 */
void LogDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & ) const {
    editor->setGeometry( option.rect );
}

/**
 * @brief LogDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void LogDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int y;
    const Id taskId = Task::instance()->id( index.row());
    bool found = false;
    const bool alternate = index.row() % 2;
    QString text( index.model()->data( index, Qt::DisplayRole ).toString());
    QPixmap pixmap;

    // get display rect
    QRect textRect( option.rect );
    textRect.setWidth( textRect.width() - 32 );

    // paint alternating rows
    if ( alternate )
        painter->fillRect( option.rect, QColor::fromRgb( 0, 0, 0, 8 ));

    // find matching log
    // not the fastest lookup, but performance is a non-issue currently
    for ( y = 0; y < Log::instance()->count(); y++ ) {
        if ( Log::instance()->taskId( y ) == taskId && Log::instance()->teamId( y ) == MainWindow::instance()->currentTeamId()) {

            // TODO: only if value > 0
            found = true;
        }
    }

    // draw task name as a multiline text
    painter->drawText( textRect, Qt::AlignVCenter | Qt::TextWordWrap, text );

    // draw button
    QRect pixmapRect( option.rect.width() - 32, option.rect.y(), 32, 32 );

    bool /*mouseOver = false,*/ u = false;
    if ( option.state & QStyle::State_MouseOver ) {
        QModelIndex under( qobject_cast<TaskView *>( this->parent())->check );
        if ( under.isValid()) {
            u = true;
            pixmap.load( found ? ":/icons/log_remove" : ":/icons/log_toggle" );
            painter->drawPixmap( pixmapRect, pixmap);
        } else {
            if ( !found ) {
                pixmap.load( ":/icons/log_unchecked" );
                painter->drawPixmap( pixmapRect, pixmap);
            } else {

            }
        }
        //mouseOver = true;
    }

    if ( found && !u ) {
        pixmap.load( ":/icons/log_checked" );
        painter->drawPixmap( pixmapRect, pixmap);
    }
}

/**
 * @brief LogDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize LogDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QSize size = QStyledItemDelegate::sizeHint( option, index );
    size.setHeight( 32 );
    return size;
}
