/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "taskview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionTeams;
    QAction *actionTasks;
    QAction *actionRankings;
    QAction *actionSettings;
    QAction *actionConsole;
    QAction *actionCombos;
    QAction *actionAddQuick;
    QAction *actionLogTime;
    QAction *actionCombine;
    QAction *actionDone;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *eventLayout;
    QLabel *eventPixmap;
    QComboBox *eventCombo;
    QHBoxLayout *teamLayout;
    QLabel *teamPixmap;
    QComboBox *teamCombo;
    TaskView *taskView;
    QHBoxLayout *horizontalLayout;
    QLabel *findPixmap;
    QLineEdit *findEdit;
    QToolButton *clearButton;
    QToolBar *toolBar;
    QToolBar *quickBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(481, 462);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/ketone"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionTeams = new QAction(MainWindow);
        actionTeams->setObjectName(QStringLiteral("actionTeams"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/teams"), QSize(), QIcon::Normal, QIcon::Off);
        actionTeams->setIcon(icon1);
        actionTasks = new QAction(MainWindow);
        actionTasks->setObjectName(QStringLiteral("actionTasks"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/tasks"), QSize(), QIcon::Normal, QIcon::Off);
        actionTasks->setIcon(icon2);
        actionRankings = new QAction(MainWindow);
        actionRankings->setObjectName(QStringLiteral("actionRankings"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/rankings"), QSize(), QIcon::Normal, QIcon::Off);
        actionRankings->setIcon(icon3);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QStringLiteral("actionSettings"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/overflow"), QSize(), QIcon::Normal, QIcon::Off);
        actionSettings->setIcon(icon4);
        actionConsole = new QAction(MainWindow);
        actionConsole->setObjectName(QStringLiteral("actionConsole"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/console"), QSize(), QIcon::Normal, QIcon::Off);
        actionConsole->setIcon(icon5);
        actionCombos = new QAction(MainWindow);
        actionCombos->setObjectName(QStringLiteral("actionCombos"));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/icons/combos"), QSize(), QIcon::Normal, QIcon::Off);
        actionCombos->setIcon(icon6);
        actionAddQuick = new QAction(MainWindow);
        actionAddQuick->setObjectName(QStringLiteral("actionAddQuick"));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/icons/add"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddQuick->setIcon(icon7);
        actionLogTime = new QAction(MainWindow);
        actionLogTime->setObjectName(QStringLiteral("actionLogTime"));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/icons/time"), QSize(), QIcon::Normal, QIcon::Off);
        actionLogTime->setIcon(icon8);
        actionCombine = new QAction(MainWindow);
        actionCombine->setObjectName(QStringLiteral("actionCombine"));
        actionCombine->setEnabled(false);
        actionCombine->setIcon(icon6);
        actionDone = new QAction(MainWindow);
        actionDone->setObjectName(QStringLiteral("actionDone"));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/icons/accept"), QSize(), QIcon::Normal, QIcon::Off);
        actionDone->setIcon(icon9);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        eventLayout = new QHBoxLayout();
        eventLayout->setSpacing(6);
        eventLayout->setObjectName(QStringLiteral("eventLayout"));
        eventPixmap = new QLabel(centralWidget);
        eventPixmap->setObjectName(QStringLiteral("eventPixmap"));
        eventPixmap->setMaximumSize(QSize(16, 16));
        eventPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/name")));

        eventLayout->addWidget(eventPixmap);

        eventCombo = new QComboBox(centralWidget);
        eventCombo->setObjectName(QStringLiteral("eventCombo"));

        eventLayout->addWidget(eventCombo);


        verticalLayout->addLayout(eventLayout);

        teamLayout = new QHBoxLayout();
        teamLayout->setSpacing(6);
        teamLayout->setObjectName(QStringLiteral("teamLayout"));
        teamPixmap = new QLabel(centralWidget);
        teamPixmap->setObjectName(QStringLiteral("teamPixmap"));
        teamPixmap->setMaximumSize(QSize(16, 16));
        teamPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/teams")));

        teamLayout->addWidget(teamPixmap);

        teamCombo = new QComboBox(centralWidget);
        teamCombo->setObjectName(QStringLiteral("teamCombo"));

        teamLayout->addWidget(teamCombo);


        verticalLayout->addLayout(teamLayout);

        taskView = new TaskView(centralWidget);
        taskView->setObjectName(QStringLiteral("taskView"));
        taskView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout->addWidget(taskView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        findPixmap = new QLabel(centralWidget);
        findPixmap->setObjectName(QStringLiteral("findPixmap"));
        findPixmap->setMaximumSize(QSize(16, 16));
        findPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/find")));

        horizontalLayout->addWidget(findPixmap);

        findEdit = new QLineEdit(centralWidget);
        findEdit->setObjectName(QStringLiteral("findEdit"));

        horizontalLayout->addWidget(findEdit);

        clearButton = new QToolButton(centralWidget);
        clearButton->setObjectName(QStringLiteral("clearButton"));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/icons/clear"), QSize(), QIcon::Normal, QIcon::Off);
        clearButton->setIcon(icon10);

        horizontalLayout->addWidget(clearButton);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setMovable(false);
        toolBar->setAllowedAreas(Qt::TopToolBarArea);
        toolBar->setIconSize(QSize(16, 16));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);
        quickBar = new QToolBar(MainWindow);
        quickBar->setObjectName(QStringLiteral("quickBar"));
        quickBar->setMovable(false);
        quickBar->setAllowedAreas(Qt::BottomToolBarArea|Qt::TopToolBarArea);
        quickBar->setIconSize(QSize(16, 16));
        quickBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        quickBar->setFloatable(false);
        MainWindow->addToolBar(Qt::BottomToolBarArea, quickBar);

        toolBar->addAction(actionTeams);
        toolBar->addAction(actionTasks);
        toolBar->addAction(actionRankings);
        toolBar->addAction(actionConsole);
        toolBar->addAction(actionCombos);
        toolBar->addAction(actionSettings);
        quickBar->addAction(actionAddQuick);
        quickBar->addAction(actionLogTime);
        quickBar->addAction(actionCombine);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Scavenger Hunt Logger", 0));
        actionTeams->setText(QApplication::translate("MainWindow", "Teams", 0));
        actionTasks->setText(QApplication::translate("MainWindow", "Tasks", 0));
        actionRankings->setText(QApplication::translate("MainWindow", "Rankings", 0));
        actionSettings->setText(QApplication::translate("MainWindow", "Settings", 0));
        actionConsole->setText(QApplication::translate("MainWindow", "Console", 0));
        actionCombos->setText(QApplication::translate("MainWindow", "Combos", 0));
        actionAddQuick->setText(QApplication::translate("MainWindow", "Quickly add team", 0));
        actionLogTime->setText(QApplication::translate("MainWindow", "Log current time", 0));
        actionCombine->setText(QApplication::translate("MainWindow", "Combine logs", 0));
        actionDone->setText(QApplication::translate("MainWindow", "Done", 0));
        eventPixmap->setText(QString());
        teamPixmap->setText(QString());
        findPixmap->setText(QString());
        clearButton->setText(QApplication::translate("MainWindow", "...", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
        quickBar->setWindowTitle(QApplication::translate("MainWindow", "quickBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
