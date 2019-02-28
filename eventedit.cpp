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
#include "editordialog.h"
#include "event.h"
#include "main.h"
#include "eventedit.h"
#include "ui_eventedit.h"
#include "variable.h"
#include "optionswidget.h"
#include "script.h"
#include <QMessageBox>

/**
 * @brief EventEdit::EventEdit
 * @param parent
 */
EventEdit::EventEdit( QWidget *parent ) : QWidget( parent ), ui( new Ui::EventEdit ), m_edit( false ) {
    // set up ui
    this->ui->setupUi( this );

    // only visible in quick add
    this->setWindowTitle( this->tr( "Add event" ));
    this->setWindowIcon( QIcon( ":/icons/ketone" ));

    /*this->addWidget( QStringList() << "bool" << "Important integer" << "true" );
    this->addWidget( QStringList() << "integer" << "Big one" << "312" );
    this->addWidget( QStringList() << "time" << "Is it time yet?" << "18:44" );
    this->addWidget( QStringList() << "string" << "Bad string" << "I'm not bad" );
    this->addWidget( QStringList() << "double" << "A double? Why?" << "1313.31" );*/

    // get event related variables
    const Row event = MainWindow::instance()->currentEvent();
    if ( event == Row::Invalid )
        return;

    // evalute current event script
    Script::instance()->evaluate( Event::instance()->script( event ));

    // call column function
    QStringList list;
    foreach ( const QVariant &var, Script::instance()->call( "options" ).toVariant().toList())
        this->addWidget( var.toString().split( ";" ));

    // empty event title check
    auto emptyTitle = [ this ]() {
        // warn upon empty event title
        if ( this->ui->titleEdit->text().isEmpty()) {
            QMessageBox::information( this, this->tr( "Empty event title" ), this->tr( "Please enter event title" ));
            return true;
        }
        return false;
    };

    // add button action
    this->connect( this->ui->addButton, &QPushButton::clicked, [ this, emptyTitle ]() {
        const QString eventTitle( this->ui->titleEdit->text());

        // abort on empty event title
        if ( emptyTitle())
            return;

        // abort on existing event
        if ( Event::instance()->contains( Event::Title, eventTitle ) && !this->isEditing()) {
            QMessageBox::information( this, this->tr( "Event already exists" ), this->tr( "Event already exists\nChoose a different title" ));
            return;
        }

        // if everything is ok, add a new event
        Row event = Row::Invalid;
        if ( !this->isEditing()) {
            event = Event::instance()->add( eventTitle, "" );
        } else {
            const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

            if ( event == Row::Invalid )
                return;

            Event::instance()->setTitle( event, eventTitle );
        }

        if ( event != Row::Invalid )
            MainWindow::instance()->setCurrentEvent( event );

        // close dock
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
    });

    // cancel button just closes the dialog
    this->connect( this->ui->cancelButton, &QPushButton::clicked, [ this ]() {
        if ( EditorDialog::instance()->isDockVisible())
            EditorDialog::instance()->hideDock();
        else
            this->close();
    } );

    // edit script button
    this->connect( this->ui->scriptButton, &QPushButton::clicked, [ this ]() {
        //Options options( this );
        //options.setAttribute( Qt::WA_DeleteOnClose, true );

        //options.exec();

        // ideally page would look like this:
        //
        // Event title: ______________________
        // list of all event variables:
        //   times,
        //   bonus points
        //   penalty
        // ( Edit Script ) -> opens a javascript editor
        //
        // making title the only hard coded variable, the rest
        //   being imported from a script
    } );

    // add to garbage man
    GarbageMan::instance()->add( this );
}

/**
 * @brief EventEdit::~EventEdit
 */
EventEdit::~EventEdit() {
    // disconnect lambdas
    this->disconnect( this->ui->addButton, SIGNAL( clicked()));
    this->disconnect( this->ui->cancelButton, SIGNAL( clicked()));
    this->disconnect( this->ui->titleEdit, SIGNAL( returnPressed()));

    // delete all widgets
    // TODO: also delete items
    qDeleteAll( this->widgets );

    // delete ui
    delete this->ui;
}

/**
 * @brief EventEdit::reset
 * @param edit
 */
void EventEdit::reset( bool edit ) {
    this->m_edit = edit;

    if ( !this->isEditing()) {
        // reset ui components to default values
        this->ui->titleEdit->clear();
   } else {
        const Row event = Event::instance()->row( EditorDialog::instance()->container->currentIndex().row());

        if ( event == Row::Invalid )
            return;

        this->ui->titleEdit->setText( Event::instance()->title( event ));
    }

    this->ui->titleEdit->setFocus();
    this->ui->addButton->setDefault( false );
    this->ui->addButton->setAutoDefault( false );
}

/**
 * @brief EventEdit::addWidget
 * @param parms
 */
void EventEdit::addWidget( const QStringList &parms ) {
    if ( parms.count() < 3 )
        return;

    // parse values
    OptionsWidget::Types type;
    const QString label( parms.at( 1 ));
    const QVariant value( QVariant( parms.at( 2 )));

    if ( !QString::compare( parms.at( 0 ), "bool" ))
        type = OptionsWidget::Bool;
    else if ( !QString::compare( parms.at( 0 ), "string" ))
        type = OptionsWidget::String;
    else if ( !QString::compare( parms.at( 0 ), "integer" ))
        type = OptionsWidget::Integer;
    else if ( !QString::compare( parms.at( 0 ), "time" ))
        type = OptionsWidget::Time;
    else if ( !QString::compare( parms.at( 0 ), "double" ))
        type = OptionsWidget::Double;
    else
        return;

    OptionsWidget *widget( new OptionsWidget( type, label, value, this ));
    QListWidgetItem *item( new QListWidgetItem( this->ui->optionList ));
    this->ui->optionList->setItemWidget( item, widget );
    widget->show();
    item->setSizeHint( widget->sizeHint());

    this->widgets << widget;
}
