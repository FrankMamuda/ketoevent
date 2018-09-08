/********************************************************************************
** Form generated from reading UI file 'console.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "console.h"

QT_BEGIN_NAMESPACE

class Ui_Console
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QTextEdit *screen;
    HistoryEdit *input;

    void setupUi(QMainWindow *Console)
    {
        if (Console->objectName().isEmpty())
            Console->setObjectName(QStringLiteral("Console"));
        Console->resize(480, 293);
        centralwidget = new QWidget(Console);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));

        verticalLayout->addLayout(horizontalLayout);

        screen = new QTextEdit(centralwidget);
        screen->setObjectName(QStringLiteral("screen"));
        screen->setStyleSheet(QStringLiteral("QTextEdit {background: #2e2f30; color: #bbbebf;}"));
        screen->setFrameShape(QFrame::NoFrame);
        screen->setReadOnly(true);
        screen->setAcceptRichText(false);

        verticalLayout->addWidget(screen);

        input = new HistoryEdit(centralwidget);
        input->setObjectName(QStringLiteral("input"));
        input->setStyleSheet(QStringLiteral("QLineEdit {background: #404244; color: #ffffff;}"));
        input->setFrame(false);
        input->setPlaceholderText(QStringLiteral(">"));

        verticalLayout->addWidget(input);

        Console->setCentralWidget(centralwidget);

        retranslateUi(Console);

        QMetaObject::connectSlotsByName(Console);
    } // setupUi

    void retranslateUi(QMainWindow *Console)
    {
        Console->setWindowTitle(QApplication::translate("Console", "Console", 0));
        input->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Console: public Ui_Console {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONSOLE_H
