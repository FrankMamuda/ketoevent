/********************************************************************************
** Form generated from reading UI file 'rankings.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RANKINGS_H
#define UI_RANKINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Rankings
{
public:
    QAction *actionUpdate;
    QAction *actionExport;
    QAction *actionCurrent;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *treamLayout;
    QLabel *teamPixmap;
    QComboBox *teamCombo;
    QTableView *tableView;
    QProgressBar *progressBar;
    QPushButton *closeButton;
    QToolBar *toolBar;

    void setupUi(QMainWindow *Rankings)
    {
        if (Rankings->objectName().isEmpty())
            Rankings->setObjectName(QStringLiteral("Rankings"));
        Rankings->resize(469, 404);
        actionUpdate = new QAction(Rankings);
        actionUpdate->setObjectName(QStringLiteral("actionUpdate"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/update"), QSize(), QIcon::Normal, QIcon::Off);
        actionUpdate->setIcon(icon);
        actionExport = new QAction(Rankings);
        actionExport->setObjectName(QStringLiteral("actionExport"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/export"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport->setIcon(icon1);
        actionCurrent = new QAction(Rankings);
        actionCurrent->setObjectName(QStringLiteral("actionCurrent"));
        actionCurrent->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/teams"), QSize(), QIcon::Normal, QIcon::Off);
        actionCurrent->setIcon(icon2);
        centralwidget = new QWidget(Rankings);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        treamLayout = new QHBoxLayout();
        treamLayout->setObjectName(QStringLiteral("treamLayout"));
        teamPixmap = new QLabel(centralwidget);
        teamPixmap->setObjectName(QStringLiteral("teamPixmap"));
        teamPixmap->setMaximumSize(QSize(16, 16));
        teamPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/teams")));

        treamLayout->addWidget(teamPixmap);

        teamCombo = new QComboBox(centralwidget);
        teamCombo->setObjectName(QStringLiteral("teamCombo"));

        treamLayout->addWidget(teamCombo);


        verticalLayout->addLayout(treamLayout);

        tableView = new QTableView(centralwidget);
        tableView->setObjectName(QStringLiteral("tableView"));
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->verticalHeader()->setVisible(false);

        verticalLayout->addWidget(tableView);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(24);

        verticalLayout->addWidget(progressBar);

        closeButton = new QPushButton(centralwidget);
        closeButton->setObjectName(QStringLiteral("closeButton"));

        verticalLayout->addWidget(closeButton);

        Rankings->setCentralWidget(centralwidget);
        toolBar = new QToolBar(Rankings);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(16, 16));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar->setFloatable(false);
        Rankings->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionUpdate);
        toolBar->addAction(actionExport);
        toolBar->addAction(actionCurrent);

        retranslateUi(Rankings);

        QMetaObject::connectSlotsByName(Rankings);
    } // setupUi

    void retranslateUi(QMainWindow *Rankings)
    {
        Rankings->setWindowTitle(QApplication::translate("Rankings", "Rankings", 0));
        actionUpdate->setText(QApplication::translate("Rankings", "Update", 0));
        actionExport->setText(QApplication::translate("Rankings", "Export", 0));
        actionCurrent->setText(QApplication::translate("Rankings", "Current team", 0));
        teamPixmap->setText(QString());
        closeButton->setText(QApplication::translate("Rankings", "Close", 0));
        toolBar->setWindowTitle(QApplication::translate("Rankings", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class Rankings: public Ui_Rankings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RANKINGS_H
