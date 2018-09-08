/********************************************************************************
** Form generated from reading UI file 'teamedit.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEAMEDIT_H
#define UI_TEAMEDIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TeamEdit
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *teamLayout;
    QLabel *reviewerLabel;
    QLineEdit *titleEdit;
    QLineEdit *reviewerEdit;
    QLabel *finishLabel;
    QTimeEdit *finishTime;
    QToolButton *finishButton;
    QLabel *membersPixmap;
    QLabel *titlePixmap;
    QSpinBox *membersInteger;
    QLabel *reviewerPixmap;
    QLabel *membersLabel;
    QLabel *titleLabel;
    QSpacerItem *horizontalSpacer;
    QPushButton *addButton;
    QPushButton *cancelButton;

    void setupUi(QWidget *TeamEdit)
    {
        if (TeamEdit->objectName().isEmpty())
            TeamEdit->setObjectName(QStringLiteral("TeamEdit"));
        TeamEdit->resize(326, 102);
        gridLayout_2 = new QGridLayout(TeamEdit);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        teamLayout = new QGridLayout();
        teamLayout->setObjectName(QStringLiteral("teamLayout"));
        reviewerLabel = new QLabel(TeamEdit);
        reviewerLabel->setObjectName(QStringLiteral("reviewerLabel"));

        teamLayout->addWidget(reviewerLabel, 1, 1, 1, 1);

        titleEdit = new QLineEdit(TeamEdit);
        titleEdit->setObjectName(QStringLiteral("titleEdit"));

        teamLayout->addWidget(titleEdit, 0, 2, 1, 1);

        reviewerEdit = new QLineEdit(TeamEdit);
        reviewerEdit->setObjectName(QStringLiteral("reviewerEdit"));

        teamLayout->addWidget(reviewerEdit, 1, 2, 1, 1);

        finishLabel = new QLabel(TeamEdit);
        finishLabel->setObjectName(QStringLiteral("finishLabel"));

        teamLayout->addWidget(finishLabel, 1, 5, 1, 1);

        finishTime = new QTimeEdit(TeamEdit);
        finishTime->setObjectName(QStringLiteral("finishTime"));

        teamLayout->addWidget(finishTime, 1, 6, 1, 1);

        finishButton = new QToolButton(TeamEdit);
        finishButton->setObjectName(QStringLiteral("finishButton"));
        finishButton->setMaximumSize(QSize(24, 24));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/time"), QSize(), QIcon::Normal, QIcon::Off);
        finishButton->setIcon(icon);

        teamLayout->addWidget(finishButton, 1, 4, 1, 1);

        membersPixmap = new QLabel(TeamEdit);
        membersPixmap->setObjectName(QStringLiteral("membersPixmap"));
        membersPixmap->setMaximumSize(QSize(16, 16));
        membersPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/teams")));
        membersPixmap->setScaledContents(true);

        teamLayout->addWidget(membersPixmap, 0, 4, 1, 1);

        titlePixmap = new QLabel(TeamEdit);
        titlePixmap->setObjectName(QStringLiteral("titlePixmap"));
        titlePixmap->setMaximumSize(QSize(16, 16));
        titlePixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/name")));

        teamLayout->addWidget(titlePixmap, 0, 0, 1, 1);

        membersInteger = new QSpinBox(TeamEdit);
        membersInteger->setObjectName(QStringLiteral("membersInteger"));

        teamLayout->addWidget(membersInteger, 0, 6, 1, 1);

        reviewerPixmap = new QLabel(TeamEdit);
        reviewerPixmap->setObjectName(QStringLiteral("reviewerPixmap"));
        reviewerPixmap->setMaximumSize(QSize(16, 16));
        reviewerPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/ketone")));
        reviewerPixmap->setScaledContents(true);

        teamLayout->addWidget(reviewerPixmap, 1, 0, 1, 1);

        membersLabel = new QLabel(TeamEdit);
        membersLabel->setObjectName(QStringLiteral("membersLabel"));

        teamLayout->addWidget(membersLabel, 0, 5, 1, 1);

        titleLabel = new QLabel(TeamEdit);
        titleLabel->setObjectName(QStringLiteral("titleLabel"));

        teamLayout->addWidget(titleLabel, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        teamLayout->addItem(horizontalSpacer, 0, 3, 2, 1);


        gridLayout_2->addLayout(teamLayout, 0, 0, 1, 2);

        addButton = new QPushButton(TeamEdit);
        addButton->setObjectName(QStringLiteral("addButton"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/accept"), QSize(), QIcon::Normal, QIcon::Off);
        addButton->setIcon(icon1);

        gridLayout_2->addWidget(addButton, 1, 0, 1, 1);

        cancelButton = new QPushButton(TeamEdit);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/remove"), QSize(), QIcon::Normal, QIcon::Off);
        cancelButton->setIcon(icon2);

        gridLayout_2->addWidget(cancelButton, 1, 1, 1, 1);


        retranslateUi(TeamEdit);

        QMetaObject::connectSlotsByName(TeamEdit);
    } // setupUi

    void retranslateUi(QWidget *TeamEdit)
    {
        TeamEdit->setWindowTitle(QApplication::translate("TeamEdit", "Form", 0));
        reviewerLabel->setText(QApplication::translate("TeamEdit", "Reviewer", 0));
        titleEdit->setPlaceholderText(QApplication::translate("TeamEdit", "Type team title here", 0));
        reviewerEdit->setPlaceholderText(QApplication::translate("TeamEdit", "Type reviewer's name here", 0));
        finishLabel->setText(QApplication::translate("TeamEdit", "Finish", 0));
        finishButton->setText(QApplication::translate("TeamEdit", "...", 0));
        membersPixmap->setText(QString());
        titlePixmap->setText(QString());
        reviewerPixmap->setText(QString());
        membersLabel->setText(QApplication::translate("TeamEdit", "Members", 0));
        titleLabel->setText(QApplication::translate("TeamEdit", "Title", 0));
        addButton->setText(QApplication::translate("TeamEdit", "Add", 0));
        cancelButton->setText(QApplication::translate("TeamEdit", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class TeamEdit: public Ui_TeamEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEAMEDIT_H
