/********************************************************************************
** Form generated from reading UI file 'combos.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMBOS_H
#define UI_COMBOS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Combos
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *comboLayout;
    QHBoxLayout *teamLayout;
    QLabel *teamPixmap;
    QLabel *teamLabel;
    QComboBox *teamCombo;
    QFrame *line;
    QHBoxLayout *titleLayout;
    QLabel *labelCombined;
    QListView *view;
    QGridLayout *statsLayout;
    QLabel *comboPixmap;
    QLabel *comboLabel;
    QLabel *pointsLabel;
    QLineEdit *combosEdit;
    QLineEdit *pointsEdit;
    QLabel *pointsPixmap;
    QPushButton *closeButton;

    void setupUi(QMainWindow *Combos)
    {
        if (Combos->objectName().isEmpty())
            Combos->setObjectName(QStringLiteral("Combos"));
        Combos->resize(471, 338);
        centralwidget = new QWidget(Combos);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        comboLayout = new QVBoxLayout();
        comboLayout->setObjectName(QStringLiteral("comboLayout"));
        teamLayout = new QHBoxLayout();
        teamLayout->setObjectName(QStringLiteral("teamLayout"));
        teamPixmap = new QLabel(centralwidget);
        teamPixmap->setObjectName(QStringLiteral("teamPixmap"));
        teamPixmap->setMaximumSize(QSize(16, 16));
        teamPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/teams")));

        teamLayout->addWidget(teamPixmap);

        teamLabel = new QLabel(centralwidget);
        teamLabel->setObjectName(QStringLiteral("teamLabel"));

        teamLayout->addWidget(teamLabel);

        teamCombo = new QComboBox(centralwidget);
        teamCombo->setObjectName(QStringLiteral("teamCombo"));

        teamLayout->addWidget(teamCombo);

        teamLayout->setStretch(1, 1);
        teamLayout->setStretch(2, 2);

        comboLayout->addLayout(teamLayout);

        line = new QFrame(centralwidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        comboLayout->addWidget(line);

        titleLayout = new QHBoxLayout();
        titleLayout->setObjectName(QStringLiteral("titleLayout"));
        labelCombined = new QLabel(centralwidget);
        labelCombined->setObjectName(QStringLiteral("labelCombined"));

        titleLayout->addWidget(labelCombined);


        comboLayout->addLayout(titleLayout);

        view = new QListView(centralwidget);
        view->setObjectName(QStringLiteral("view"));

        comboLayout->addWidget(view);

        statsLayout = new QGridLayout();
        statsLayout->setObjectName(QStringLiteral("statsLayout"));
        comboPixmap = new QLabel(centralwidget);
        comboPixmap->setObjectName(QStringLiteral("comboPixmap"));
        comboPixmap->setMaximumSize(QSize(16, 16));
        comboPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/combos")));

        statsLayout->addWidget(comboPixmap, 0, 0, 1, 1);

        comboLabel = new QLabel(centralwidget);
        comboLabel->setObjectName(QStringLiteral("comboLabel"));

        statsLayout->addWidget(comboLabel, 0, 1, 1, 1);

        pointsLabel = new QLabel(centralwidget);
        pointsLabel->setObjectName(QStringLiteral("pointsLabel"));

        statsLayout->addWidget(pointsLabel, 0, 4, 1, 1);

        combosEdit = new QLineEdit(centralwidget);
        combosEdit->setObjectName(QStringLiteral("combosEdit"));
        combosEdit->setReadOnly(true);

        statsLayout->addWidget(combosEdit, 0, 2, 1, 1);

        pointsEdit = new QLineEdit(centralwidget);
        pointsEdit->setObjectName(QStringLiteral("pointsEdit"));
        pointsEdit->setReadOnly(true);

        statsLayout->addWidget(pointsEdit, 0, 5, 1, 1);

        pointsPixmap = new QLabel(centralwidget);
        pointsPixmap->setObjectName(QStringLiteral("pointsPixmap"));
        pointsPixmap->setMaximumSize(QSize(16, 16));
        pointsPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/star")));

        statsLayout->addWidget(pointsPixmap, 0, 3, 1, 1);


        comboLayout->addLayout(statsLayout);

        comboLayout->setStretch(3, 1);

        verticalLayout->addLayout(comboLayout);

        closeButton = new QPushButton(centralwidget);
        closeButton->setObjectName(QStringLiteral("closeButton"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/close.png"), QSize(), QIcon::Normal, QIcon::Off);
        closeButton->setIcon(icon);
        closeButton->setDefault(true);

        verticalLayout->addWidget(closeButton);

        Combos->setCentralWidget(centralwidget);

        retranslateUi(Combos);

        QMetaObject::connectSlotsByName(Combos);
    } // setupUi

    void retranslateUi(QMainWindow *Combos)
    {
        Combos->setWindowTitle(QApplication::translate("Combos", "Combinations", 0));
        teamPixmap->setText(QString());
        teamLabel->setText(QApplication::translate("Combos", "Team", 0));
        labelCombined->setText(QApplication::translate("Combos", "Combined tasks:", 0));
        comboPixmap->setText(QString());
        comboLabel->setText(QApplication::translate("Combos", "Combinations", 0));
        pointsLabel->setText(QApplication::translate("Combos", "Bonus points", 0));
        pointsPixmap->setText(QString());
        closeButton->setText(QApplication::translate("Combos", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class Combos: public Ui_Combos {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMBOS_H
