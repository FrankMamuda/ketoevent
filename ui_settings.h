/********************************************************************************
** Form generated from reading UI file 'settings.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Settings
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *reviewerLayout;
    QLabel *reviewerPixmap;
    QLineEdit *reviewerEdit;
    QCheckBox *sortByTypeCheck;
    QPushButton *closeButton;

    void setupUi(QMainWindow *Settings)
    {
        if (Settings->objectName().isEmpty())
            Settings->setObjectName(QStringLiteral("Settings"));
        Settings->resize(253, 92);
        centralwidget = new QWidget(Settings);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        reviewerLayout = new QHBoxLayout();
        reviewerLayout->setObjectName(QStringLiteral("reviewerLayout"));
        reviewerPixmap = new QLabel(centralwidget);
        reviewerPixmap->setObjectName(QStringLiteral("reviewerPixmap"));
        reviewerPixmap->setMaximumSize(QSize(16, 16));
        reviewerPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/ketone")));
        reviewerPixmap->setScaledContents(true);

        reviewerLayout->addWidget(reviewerPixmap);

        reviewerEdit = new QLineEdit(centralwidget);
        reviewerEdit->setObjectName(QStringLiteral("reviewerEdit"));

        reviewerLayout->addWidget(reviewerEdit);


        verticalLayout->addLayout(reviewerLayout);

        sortByTypeCheck = new QCheckBox(centralwidget);
        sortByTypeCheck->setObjectName(QStringLiteral("sortByTypeCheck"));

        verticalLayout->addWidget(sortByTypeCheck);

        closeButton = new QPushButton(centralwidget);
        closeButton->setObjectName(QStringLiteral("closeButton"));

        verticalLayout->addWidget(closeButton);

        Settings->setCentralWidget(centralwidget);

        retranslateUi(Settings);

        QMetaObject::connectSlotsByName(Settings);
    } // setupUi

    void retranslateUi(QMainWindow *Settings)
    {
        Settings->setWindowTitle(QApplication::translate("Settings", "Settings", 0));
        reviewerPixmap->setText(QString());
        reviewerEdit->setPlaceholderText(QApplication::translate("Settings", "Type reviewer name here", 0));
        sortByTypeCheck->setText(QApplication::translate("Settings", "Sort tasks by type", 0));
        closeButton->setText(QApplication::translate("Settings", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class Settings: public Ui_Settings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGS_H
