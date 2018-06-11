#include "logeditor.h"
#include "ui_logeditor.h"
#include <QDebug>

LogEditor::LogEditor(QWidget *parent) :
    QWidget(parent)/*,
    ui(new Ui::LogEditor)*/
{
    //ui->setupUi(this);

    qDebug() << "aaa";
    this->setFixedHeight( 32 );
    QPushButton *button = new QPushButton( this );
    Q_UNUSED( button )

}

LogEditor::~LogEditor()
{
    //delete ui;
}

void LogEditor::setName(const QString &name)
{
    Q_UNUSED( name )

    //this->ui->taskName->setText( name );
}
