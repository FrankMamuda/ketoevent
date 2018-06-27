/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "main.h"
#include "modalwindow.h"
#include <QLineEdit>
#include <QMouseEvent>

#ifdef QT_DEBUG

//
// namespace: Ui
//
namespace Ui {
class Console;
#ifdef Q_CC_MSVC
static const int MaxConsoleHistory = 32;
#else
static const int __attribute__((unused)) MaxConsoleHistory = 32;
#endif
}

//
// classes
//
class HistoryEdit;

/**
 * @brief The Console class
 */
class Console final : public ModalWindow {
    Q_OBJECT
    Q_DISABLE_COPY( Console )

public:
    static Console *instance() { static Console *instance( new Console()); GarbageMan::instance()->add( instance ); return instance; }
    virtual ~Console();

public slots:
    void print( const QString &msg );
    bool completeCommand();

protected:
    void mousePressEvent( QMouseEvent *event ) override { this->m_windowPos = event->pos(); }
    void mouseMoveEvent( QMouseEvent *event ) override { if ( event->buttons() && Qt::LeftButton ) this->move( this->pos() + event->pos() - this->m_windowPos ); }
    bool eventFilter( QObject *object, QEvent *event ) override;

private slots:
    void on_input_returnPressed();

private:
    explicit Console();
    Ui::Console *ui;
    QPoint m_windowPos;
    HistoryEdit *edit;
};

/**
 * @brief The HistoryEdit class
 */
class HistoryEdit final : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY( int offset READ offset WRITE set RESET reset )
    friend Console;

public:
    explicit HistoryEdit( QWidget *parent = nullptr ) : m_historyOffset( 0 ) { this->setParent( parent ); this->reset(); }
    virtual ~HistoryEdit() { this->history.clear(); }
    int offset() const { return this->m_historyOffset; }

public slots:
    void set( int offset = 0 ) { this->m_historyOffset = offset; }
    void reset() { this->set(); }
    void push() { this->m_historyOffset++; }
    void pop() { this->m_historyOffset--; }
    void add( const QString &text ) {
        if ( this->history.count()) {
            if ( !QString::compare( this->history.last(), text ))
                return;
        }

        if ( this->history.count() >= Ui::MaxConsoleHistory )
            this->history.removeFirst();

        this->history << text;
    }

private:
    int m_historyOffset;
    QStringList history;
};

#endif
