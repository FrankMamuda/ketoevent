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

//
// includes
//
#include "taskwidget.h"
#include "main.h"
#include <QSqlQuery>
#include "mainwindow.h"
#include <QPainter>
#include "description.h"

/**
 * @brief TaskWidget::TaskWidget
 * @param parentPtr
 */
TaskWidget::TaskWidget( Task *parentPtr ) {
    QFont font;

    // reset widgets
    this->m_log = nullptr;
    this->check = nullptr;
    this->multi = nullptr;

    // set task
    this->setTask( parentPtr );

    // failsafe
    if ( !this->hasTask()) {
        Common::error( StrSoftError, this->tr( "task not set\n" ));
        return;
    }

    // create widget
    this->taskLayout = new QHBoxLayout();
    this->taskName = new QLabel( this->task()->name());
    this->comboIcon = new QLabel();
    this->taskLayout->addWidget( this->taskName );

    // create spacer
    this->spacer = new QSpacerItem( 0, 0, QSizePolicy::Maximum, QSizePolicy::Minimum );
    this->taskLayout->addSpacerItem( this->spacer );

#ifdef UI_INFO_BUTTON_ENABLED
    // add info button
    this->info = new QPushButton( QIcon( ":/icons/about" ), "" );
    this->info->setMaximumWidth( 32 );
    this->info->setFlat( true );

    if ( !this->task()->description().isEmpty()) {
        this->taskLayout->addWidget( this->info );
    }
    this->connect( this->info, SIGNAL( clicked( bool )), this, SLOT( displayInfo()));
#endif

    this->taskLayout->addWidget( this->comboIcon );
    this->comboIcon->setMaximumSize( 16, 16 );

    // fix ugly labels/spinboxes
    this->taskLayout->setMargin( 0 );
    this->taskLayout->setSizeConstraint( QLayout::SetMinimumSize );

    // get font
    font = this->taskName->font();

    // set appropriate font
    if ( this->task()->style() == Task::Italic ) {
        font.setItalic( true );
        this->taskName->setFont( font );
    } else if ( this->task()->style() == Task::Bold ) {
        font.setBold( true );
    }

    // set font & tooltip
    this->taskName->setFont( font );
    this->taskName->setToolTip( this->task()->description());

    // determine type
    if ( this->task()->type() == Task::Check ) {
        this->check = new QCheckBox();
        this->check->setMaximumWidth( 48 );
        this->check->setGeometry( this->check->x(), this->check->y(), this->check->width(), this->check->height() * 2 );

        // changed from click for compatibility with the stressTest
        this->connect( this->check, SIGNAL( stateChanged( int )), this, SLOT( saveLog()));
        this->taskLayout->addWidget( this->check );

        // set tooltips
        //this->taskName->setToolTip( this->tr( "%1 points, click checkbox to enable/disable" ).arg( this->task()->points()));
        this->check->setToolTip( this->tr( "Toggle task completion" ));
    } else if ( this->task()->type() == Task::Multi ) {
        this->multi = new QSpinBox();
        this->multi->setMaximumWidth( 64 );

        if ( this->task()->type() == Task::Multi )
            this->multi->setMaximum( this->task()->multi());

        this->connect( this->multi, SIGNAL( valueChanged( int )), this, SLOT( saveLog()));
        this->taskLayout->addWidget( this->multi );

        // set tooltips
        //this->taskName->setToolTip( this->tr( "%1 points (max %2), multiplied by value" ).arg( this->task()->points()).arg( this->task()->multi()));
        this->multi->setToolTip( this->tr( "Change task multiplier" ));
    } else {
        Common::error( StrSoftError, this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set up combo button
    this->combo = new QPushButton( QIcon( ":/icons/add.png" ), "" );
    this->combo->setMaximumWidth( 32 );

    // set tooltips
    //this->combo->setToolTip( this->tr( "Click here to toggle combo state" ));

    // disable combo button by default
    this->combo->setCheckable( true );

    // add layout to widget
    this->setLayout( taskLayout );

    // connect combo button for updates
    this->connect( this->combo, SIGNAL( toggled( bool )), this, SLOT( toggleCombo( bool )));
}

/**
 * @brief TaskWidget::toggleViewState
 * @param state
 */
void TaskWidget::toggleViewState( ViewState state ) {
    // failsafe
    if ( !this->hasTask()) {
        Common::error( StrSoftError, this->tr( "task not set\n" ));
        return;
    }

    switch ( state ) {
    case LogView:
        this->taskLayout->removeWidget( this->combo );
        this->combo->setParent( nullptr );
#ifdef UI_INFO_BUTTON_ENABLED
        if ( !this->task()->description().isEmpty())
            this->taskLayout->addWidget( this->info );
#endif
        if ( this->task()->type() == Task::Check ) {
            this->taskLayout->addWidget( this->check );
        } else if ( this->task()->type() == Task::Multi ) {
            this->taskLayout->addWidget( this->multi );
        }
        break;

    case Combine:
#ifdef UI_INFO_BUTTON_ENABLED
        if ( !this->task()->description().isEmpty()) {
            this->taskLayout->removeWidget( this->info );
            this->info->setParent( nullptr );
        }
#endif
        if ( this->task()->type() == Task::Check ) {
            this->taskLayout->removeWidget( this->check );
            this->check->setParent( nullptr );
        } else if ( this->task()->type() == Task::Multi ) {
            this->taskLayout->removeWidget( this->multi );
            this->multi->setParent( nullptr );
        }
        this->taskLayout->addWidget( this->combo );
        break;

    case NoState:
        Common::error( StrSoftError, this->tr( "invalid view state\n" ));
        break;
    }
}

/**
 * @brief TaskWidget::saveLog
 */
void TaskWidget::saveLog() {
    int value = 0;

    if ( !this->isActive())
        return;

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        Common::error( StrFatalError, this->tr( "task or team not set\n" ));
        return;
    }

    // determine type
    if ( this->task()->type() == Task::Check ) {
        if ( this->check->checkState() == Qt::Checked )
            value = true;
        else
            value = false;
    } else if ( this->task()->type() == Task::Multi ) {
        value = this->multi->value();
    } else {
        Common::error( StrFatalError, this->tr( "invalid task type \"%1\"\n" ).arg( static_cast<int>( this->task()->type())));
        return;
    }

    // set to zero
    if ( value <= 0 && this->hasLog()) {
        // we don't really care if there are multiple logs, this orphaned log will be removed on next start anyway
        this->log()->setValue( 0 );

        // remove orphaned combos
        int count = 0;
        int badId = this->log()->comboId();
        foreach ( Log *log, Main::instance()->logList ) {
            if ( log->comboId() == badId )
                count++;
        }
        if ( count == 2 ) {
            foreach ( Log *log, Main::instance()->logList ) {
                if ( log->comboId() == badId )
                    log->setComboId( -1 );
            }
        }

        // this must be done here
        this->log()->setComboId( -1 );

        // make sure to assign new log on next value change, so invalidate this one
        this->resetLog();
    }

    // no log?, no problem - create one
    if ( !this->hasLog()) {
        // check for local logs
        // set log
        Log *log = Log::forIds( this->team()->id(), this->task()->id());
        if ( log != nullptr ) {
            this->setLog( log, false );
        } else {
            // add new log
            log = Log::add( this->task()->id(), this->team()->id());
            this->setLog( log, true );

            // FIXME: why here and in log.cpp?
            if ( !this->team()->logList.contains( log ))
                this->team()->logList << this->log();
        }
    }
    // update THIS and only THIS log
    this->log()->setValue( value );

    MainWindow *mainWindow = qobject_cast<MainWindow *>( Main::instance()->parent());
    if ( mainWindow != nullptr )
        mainWindow->taskIndexChanged( -1 );

    // changes made - must recalculate combos
    this->team()->setCombosCalculated( false );

    if ( value > 0 && Variable::instance()->isEnabled( "misc/hilightLogged" ))
        this->taskName->setForegroundRole( QPalette::Highlight );
    else
        this->taskName->setForegroundRole( QPalette::NoRole );
}

/**
 * @brief TaskWidget::~TaskWidget
 */
TaskWidget::~TaskWidget() {
    // failsafe
    if ( !this->hasTask()) {
        Common::error( StrSoftError, this->tr( "task not set\n" ));
        return;
    }

    // disconnect buttons and spinboxes
    if ( this->task()->type() == Task::Check && this->check != nullptr ) {
        this->disconnect( this->check, SIGNAL( clicked()));
        delete this->check;
    } else if ( this->task()->type() == Task::Multi && this->multi != nullptr ) {
        this->disconnect( this->multi, SIGNAL( valueChanged( int )));
        delete this->multi;
    }

    // clean up
    if ( this->taskName != nullptr )
        delete this->taskName;

    if ( this->comboIcon != nullptr )
        delete this->comboIcon;


        delete this->combo;

#ifdef UI_INFO_BUTTON_ENABLED
    if ( this->info != nullptr )
        delete this->info;
#endif

    if ( this->taskLayout != nullptr )
        delete this->taskLayout;
}

/**
 * @brief TaskWidget::setLog
 * @param log
 * @param fromDatabase
 */
void TaskWidget::setLog( Log *log, bool fromDatabase ) {
    this->m_log = log;

    // failsafe
    if ( !this->hasTask()) {
        Common::error( StrSoftError, this->tr( "task not set\n" ));
        return;
    }

    // failsafe
    if ( !this->hasLog()) {
        Common::error( StrSoftError, this->tr( "log not set\n" ));
        return;
    }

    // failsafe
    if ( !this->hasTeam()) {
        Common::error( StrSoftError, this->tr( "team not set\n" ));
        return;
    }

    Task *task = Task::forId( log->taskId());
    if ( task == nullptr )
        return;

    // make sure log's task is the same task
    if ( task != this->task())
        return;

    // connect for id updates
    this->connect( this->log(), SIGNAL( comboIdChanged()), this, SLOT( comboIdChanged()));

    // trigger id change
    this->comboIdChanged();

    // just initial addition
    if ( fromDatabase )
        return;

    // set values
    if ( this->task()->type() == Task::Check )
        this->check->setChecked( this->log()->check());
    else if ( this->task()->type() == Task::Multi )
        this->multi->setValue( this->log()->value());

    if ( this->log()->value() > 0 && Variable::instance()->isEnabled( "misc/hilightLogged" ))
        this->taskName->setForegroundRole( QPalette::Highlight );
    else
        this->taskName->setForegroundRole( QPalette::NoRole );
}

/**
 * @brief TaskWidget::getRelativeComboId
 * @param comboId
 * @param teamId
 * @return
 */
int TaskWidget::getRelativeComboId( int comboId, int teamId ) {
    Team *team = Team::forId( teamId );
    if ( team == nullptr )
        return -1;

    QList <int>combos;
    foreach ( Log *log, team->logList ) {
        int id = log->comboId();

        if ( id == -1 )
            continue;

        if ( !combos.contains( id ))
            combos << id;
    }
    return combos.indexOf( comboId ) + 1;
}

/**
 * @brief TaskWidget::comboIdChanged
 */
void TaskWidget::comboIdChanged() {
    if ( this->hasTeam() && this->hasCombo()) {
        QPixmap px( ":/icons/combo.png" );
        QPainter painter( &px );
        QFont font( "Times New Roman" );
        font.setPointSize( 32 );
        font.setBold( true );
        painter.setFont( font );
        painter.drawText( px.rect(), Qt::AlignCenter, QString( "%1" ).arg( this->getRelativeComboId( this->log()->comboId(), this->team()->id())));
        this->comboIcon->setPixmap( px );
        this->comboIcon->setScaledContents( true );
    } else
        this->comboIcon->setPixmap( QPixmap());
}

/**
 * @brief TaskWidget::resetLog
 */
void TaskWidget::resetLog() {
    this->m_log = nullptr;

    // failsafe - this really should not happen
    if ( !this->hasTask()) {
        Common::error( StrFatalError, this->tr( "task not set\n" ));
        return;
    }

    // reset values
    if ( this->task()->type() == Task::Check )
        this->check->setChecked( false );
    if ( this->task()->type() == Task::Multi )
        this->multi->setValue( 0 );
}

/**
 * @brief TaskWidget::setTeam
 * @param team
 */
void TaskWidget::setTeam( Team *team ) {
    this->m_team = team;

    // failsafe
    if ( !this->hasTask() || !this->hasTeam()) {
        Common::error( StrSoftError, this->tr( "task, team or log not set\n" ));
        return;
    }

    // find appropriate log
    foreach ( Log *log, team->logList ) {
        Task *task = Task::forId( log->taskId());

        if ( task == nullptr )
            continue;

        if ( task == this->task()) {
            this->setLog( log );
            break;
        }
    }

    // enable
    this->setActive();
}

/**
 * @brief TaskWidget::resetTeam
 */
void TaskWidget::resetTeam() {
    this->m_team = nullptr;
    this->setActive( false );
    this->resetLog();

    // reset hilights
    this->taskName->setForegroundRole( QPalette::NoRole );
}

/**
 * @brief TaskWidget::toggleCombo
 * @param checked
 */
void TaskWidget::toggleCombo( bool checked ) {    
    MainWindow *mainWindow;

    mainWindow = qobject_cast<MainWindow *>( Main::instance()->parent());
    if ( mainWindow == nullptr )
        return;

    if ( checked ) {
        this->comboIcon->hide();
        this->taskName->setStyleSheet( "background-color: rgba( 0, 200, 0, 128 ); border-style: outset; border-width: 2px; border-radius: 10px; border-color: beige; font: bold; padding: 6px;" );
        this->combo->setIcon( QIcon( ":/icons/remove.png" ));

        if ( !this->hasLog() || this->hasCombo() || !this->hasTeam())
            return;

        this->log()->setComboId( mainWindow->currentComboIndex());

        // changes made - must recalculate combos
        this->team()->setCombosCalculated( false );
    } else {
        this->comboIcon->show();
        this->taskName->setStyleSheet( "padding: 6px;" );
        this->combo->setIcon( QIcon( ":/icons/add.png" ));

        if ( !this->hasLog() && mainWindow->currentComboIndex() != -1 )
            return;

        if ( !this->hasTeam())
            return;

        this->log()->setComboId( -1 );
        this->disconnect( this->log(), SIGNAL( comboIdChanged()));

        // changes made - must recalculate combos
        this->team()->setCombosCalculated( false );
    }
}

#ifdef UI_INFO_BUTTON_ENABLED
/**
 * @brief TaskWidget::displayInfo
 */
void TaskWidget::displayInfo() {
    Description desc( this->task(), this );
    desc.exec();
}
#endif
