#include "gui_combos.h"
#include "ui_gui_combos.h"
#include "gui_main.h"



/*
================
listToAscending
================
*/
bool listByCombos( LogEntry *lPtr0, LogEntry *lPtr1 ) {
    return lPtr0->comboId() < lPtr1->comboId();
}


//rgba( 0, 200, 0, 128 )
// table for colours?
//
// some 5-10 shades and a function to assign
// colour by comboId (relative)
//



Gui_Combos::Gui_Combos( QWidget *parent ) : QDialog( parent ), ui( new Ui::Gui_Combos ) {
    ui->setupUi(this);
    this->fillTeams();

    TeamEntry *teamPtr = m.teamForId( this->ui->comboTeams->itemData( this->ui->comboTeams->currentIndex()).toInt());
    if ( teamPtr == NULL )
        return;


    // make a local copy and sort it alphabetically
    this->logListSorted = teamPtr->logList;
    qSort( this->logListSorted.begin(), this->logListSorted.end(), listByCombos );
    //this->logListSorted.removeAll( -1 );
    foreach ( LogEntry *logPtr, this->logListSorted ) {
        if ( logPtr->comboId() == -1 )
            this->logListSorted.removeOne( logPtr );
    }

    //this->ui->listCombined->add

    //logListSorted = m.


    // set up view
    this->comboModelPtr = new Gui_ComboModel( this );
    m.print( QString( "num combo tasks %1\n" ).arg( this->logListSorted.count()));
    this->ui->listCombined->setModel( this->comboModelPtr );


   /* TODO: next draw these as a rainbow as in
        task1 \
        task2 - combo 1 - green
        task3 /
        task4 - combo 2 - red
        task5 /
        etc.

        to visualize all combos in distinct (pre-coded?) colours
    */

    // set up view
    /*this->listModelPtr = new Gui_TaskListModel( this );
    this->ui->taskList->setModel( this->listModelPtr );
    this->ui->taskList->setAlternatingRowColors( true );
    this->ui->taskList->setSelectionMode( QAbstractItemView::SingleSelection );
    this->ui->taskMaxMulti->setMinimum( 2 );*/

    //this->ui->listAvailable->setModel( this->comboModelPtr );
    //this->ui->listAvailable->setAlternatingRowColors( true );

    // current team
}

Gui_Combos::~Gui_Combos() {
    this->logListSorted.clear();
    delete this->comboModelPtr;
    delete ui;
}

/*
================
fillTeams
================
*/
void Gui_Combos::fillTeams() {
#if 1
    // abort if partially initialized
    if ( !m.isInitialized())
        return;

    // clear list
    this->ui->comboTeams->clear();

    // repopulate list
    foreach ( TeamEntry *teamPtr, m.teamList )
        this->ui->comboTeams->addItem( teamPtr->name(), teamPtr->id());

    // set to current team
    Gui_Main *mPtr = qobject_cast<Gui_Main*>( this->parent());
    this->ui->comboTeams->setCurrentIndex( mPtr->currentTeamIndex());
#endif
}
