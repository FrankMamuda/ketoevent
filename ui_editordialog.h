/********************************************************************************
** Form generated from reading UI file 'editordialog.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITORDIALOG_H
#define UI_EDITORDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "dockwidget.h"

QT_BEGIN_NAMESPACE

class Ui_EditorDialog
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QListView *listView;
    QPushButton *buttonClose;
    DockWidget *dockWidget;
    QWidget *dockWidgetContents;

    void setupUi(QMainWindow *EditorDialog)
    {
        if (EditorDialog->objectName().isEmpty())
            EditorDialog->setObjectName(QStringLiteral("EditorDialog"));
        EditorDialog->resize(480, 640);
        centralwidget = new QWidget(EditorDialog);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        listView = new QListView(centralwidget);
        listView->setObjectName(QStringLiteral("listView"));
        listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listView->setDefaultDropAction(Qt::IgnoreAction);
        listView->setAlternatingRowColors(true);
        listView->setSelectionBehavior(QAbstractItemView::SelectRows);
        listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        verticalLayout->addWidget(listView);

        buttonClose = new QPushButton(centralwidget);
        buttonClose->setObjectName(QStringLiteral("buttonClose"));

        verticalLayout->addWidget(buttonClose);

        EditorDialog->setCentralWidget(centralwidget);
        dockWidget = new DockWidget(EditorDialog);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidget->setFeatures(QDockWidget::DockWidgetClosable);
        dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        dockWidget->setWidget(dockWidgetContents);
        EditorDialog->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidget);

        retranslateUi(EditorDialog);

        QMetaObject::connectSlotsByName(EditorDialog);
    } // setupUi

    void retranslateUi(QMainWindow *EditorDialog)
    {
        EditorDialog->setWindowTitle(QApplication::translate("EditorDialog", "MainWindow", 0));
        buttonClose->setText(QApplication::translate("EditorDialog", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class EditorDialog: public Ui_EditorDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITORDIALOG_H
