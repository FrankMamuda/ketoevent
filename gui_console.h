/*
 * Copyright (C) 2013-2016 Avotu Briezhaudzetava
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

#ifndef GUI_CONSOLE_H
#define GUI_CONSOLE_H

//
// defines
//
#define APPLET_DEBUG

//
// includes
//
#include <QDialog>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLineEdit>

#ifdef APPLET_DEBUG
#include <QDebug>

//
// namespace: Ui
//
namespace Ui {
class Gui_Console;
static const int MaxConsoleHistory = 32;
}

/**
 * @brief The ConsoleEventFilter class
 */
class ConsoleEventFilter : public QObject {
public:
    ConsoleEventFilter( QObject *parent = 0 ) : QObject( parent ) {}

protected:
    virtual bool eventFilter( QObject *, QEvent * );
};
#endif

/**
 * @brief The QConsoleEdit class
 */
class QConsoleEdit : public QLineEdit {
    Q_OBJECT
#ifdef APPLET_DEBUG
    Q_PROPERTY( int historyOffset READ historyOffset WRITE setHistoryOffset RESET resetHistoryOffset )

public:
    QConsoleEdit( QWidget *parent = 0 ) { this->setParent( parent ); this->resetHistoryOffset(); }
    int historyOffset() const { return this->m_historyOffset; }
    QStringList history;
    ~QConsoleEdit() { this->history.clear(); }

public slots:
    void setHistoryOffset( int offset ) { this->m_historyOffset = offset; }
    void resetHistoryOffset() { this->m_historyOffset = 0; }
    void pushHistoryOffset() { this->m_historyOffset++; }
    void popHistoryOffset() { this->m_historyOffset--; }
    void addToHistory( const QString &text ) {
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
#else
public:
    QConsoleEdit( QWidget *parent = 0 ) { Q_UNUSED( parent ) }
    ~QConsoleEdit() { }
#endif
};

#ifdef APPLET_DEBUG

/**
 * @brief The Gui_Console class
 */
class Gui_Console : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Debugging console" )

public:
    explicit Gui_Console( QWidget *parent = 0 );
    ~Gui_Console();

public slots:
    void print( const QString &msg );
    bool completeCommand();
    void loadHistory();
    void saveHisotry();

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );

private slots:
    void on_input_returnPressed();

private:
    Ui::Gui_Console *ui;
    QPoint m_windowPos;
    ConsoleEventFilter *eventFilter;
};

#endif
#endif // GUI_CONSOLE_H
