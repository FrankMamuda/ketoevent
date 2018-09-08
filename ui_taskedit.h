/********************************************************************************
** Form generated from reading UI file 'taskedit.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TASKEDIT_H
#define UI_TASKEDIT_H

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
#include <QtWidgets/QWidget>
#include "combobox.h"

QT_BEGIN_NAMESPACE

class Ui_TaskEdit
{
public:
    QGridLayout *gridLayout;
    QGridLayout *taskLayout;
    QLabel *namePixmap;
    QLabel *nameLabel;
    QLabel *pointsLabel;
    QSpinBox *pointsInteger;
    QLabel *multiPixmap;
    QLabel *multiLabel;
    QSpinBox *multiInteger;
    QLabel *typeLabel;
    ComboBox *typeCombo;
    QLabel *stylePixmap;
    QLabel *styleLabel;
    ComboBox *styleCombo;
    QLabel *descPixmap;
    QLabel *descLabel;
    QLineEdit *nameEdit;
    QLineEdit *descEdit;
    QSpacerItem *optionsSpacer;
    QLabel *typePixmap;
    QLabel *pointsPixmap;
    QPushButton *addButton;
    QPushButton *cancelButton;

    void setupUi(QWidget *TaskEdit)
    {
        if (TaskEdit->objectName().isEmpty())
            TaskEdit->setObjectName(QStringLiteral("TaskEdit"));
        TaskEdit->resize(410, 157);
        gridLayout = new QGridLayout(TaskEdit);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        taskLayout = new QGridLayout();
        taskLayout->setObjectName(QStringLiteral("taskLayout"));
        namePixmap = new QLabel(TaskEdit);
        namePixmap->setObjectName(QStringLiteral("namePixmap"));
        namePixmap->setMaximumSize(QSize(16, 16));
        namePixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/name")));

        taskLayout->addWidget(namePixmap, 0, 0, 1, 1);

        nameLabel = new QLabel(TaskEdit);
        nameLabel->setObjectName(QStringLiteral("nameLabel"));

        taskLayout->addWidget(nameLabel, 0, 1, 1, 1);

        pointsLabel = new QLabel(TaskEdit);
        pointsLabel->setObjectName(QStringLiteral("pointsLabel"));

        taskLayout->addWidget(pointsLabel, 1, 1, 1, 1);

        pointsInteger = new QSpinBox(TaskEdit);
        pointsInteger->setObjectName(QStringLiteral("pointsInteger"));
        pointsInteger->setMinimum(1);
        pointsInteger->setMaximum(999);

        taskLayout->addWidget(pointsInteger, 1, 2, 1, 1);

        multiPixmap = new QLabel(TaskEdit);
        multiPixmap->setObjectName(QStringLiteral("multiPixmap"));
        multiPixmap->setMaximumSize(QSize(16, 16));
        multiPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/multi")));

        taskLayout->addWidget(multiPixmap, 1, 4, 1, 1);

        multiLabel = new QLabel(TaskEdit);
        multiLabel->setObjectName(QStringLiteral("multiLabel"));

        taskLayout->addWidget(multiLabel, 1, 5, 1, 1);

        multiInteger = new QSpinBox(TaskEdit);
        multiInteger->setObjectName(QStringLiteral("multiInteger"));
        multiInteger->setMinimum(1);
        multiInteger->setMaximum(999);
        multiInteger->setValue(1);

        taskLayout->addWidget(multiInteger, 1, 6, 1, 1);

        typeLabel = new QLabel(TaskEdit);
        typeLabel->setObjectName(QStringLiteral("typeLabel"));

        taskLayout->addWidget(typeLabel, 2, 1, 1, 1);

        typeCombo = new ComboBox(TaskEdit);
        typeCombo->setObjectName(QStringLiteral("typeCombo"));
        typeCombo->setMaxCount(10);

        taskLayout->addWidget(typeCombo, 2, 2, 1, 1);

        stylePixmap = new QLabel(TaskEdit);
        stylePixmap->setObjectName(QStringLiteral("stylePixmap"));
        stylePixmap->setMaximumSize(QSize(16, 16));
        stylePixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/style")));

        taskLayout->addWidget(stylePixmap, 2, 4, 1, 1);

        styleLabel = new QLabel(TaskEdit);
        styleLabel->setObjectName(QStringLiteral("styleLabel"));

        taskLayout->addWidget(styleLabel, 2, 5, 1, 1);

        styleCombo = new ComboBox(TaskEdit);
        styleCombo->setObjectName(QStringLiteral("styleCombo"));

        taskLayout->addWidget(styleCombo, 2, 6, 1, 1);

        descPixmap = new QLabel(TaskEdit);
        descPixmap->setObjectName(QStringLiteral("descPixmap"));
        descPixmap->setMaximumSize(QSize(16, 16));
        descPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/info")));

        taskLayout->addWidget(descPixmap, 3, 0, 1, 1);

        descLabel = new QLabel(TaskEdit);
        descLabel->setObjectName(QStringLiteral("descLabel"));

        taskLayout->addWidget(descLabel, 3, 1, 1, 1);

        nameEdit = new QLineEdit(TaskEdit);
        nameEdit->setObjectName(QStringLiteral("nameEdit"));

        taskLayout->addWidget(nameEdit, 0, 2, 1, 5);

        descEdit = new QLineEdit(TaskEdit);
        descEdit->setObjectName(QStringLiteral("descEdit"));

        taskLayout->addWidget(descEdit, 3, 2, 1, 5);

        optionsSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        taskLayout->addItem(optionsSpacer, 1, 3, 2, 1);

        typePixmap = new QLabel(TaskEdit);
        typePixmap->setObjectName(QStringLiteral("typePixmap"));
        typePixmap->setMaximumSize(QSize(16, 16));
        typePixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/type")));

        taskLayout->addWidget(typePixmap, 2, 0, 1, 1);

        pointsPixmap = new QLabel(TaskEdit);
        pointsPixmap->setObjectName(QStringLiteral("pointsPixmap"));
        pointsPixmap->setMaximumSize(QSize(16, 16));
        pointsPixmap->setPixmap(QPixmap(QString::fromUtf8(":/icons/star")));

        taskLayout->addWidget(pointsPixmap, 1, 0, 1, 1);


        gridLayout->addLayout(taskLayout, 0, 0, 1, 2);

        addButton = new QPushButton(TaskEdit);
        addButton->setObjectName(QStringLiteral("addButton"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/accept"), QSize(), QIcon::Normal, QIcon::Off);
        addButton->setIcon(icon);

        gridLayout->addWidget(addButton, 1, 0, 1, 1);

        cancelButton = new QPushButton(TaskEdit);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/remove"), QSize(), QIcon::Normal, QIcon::Off);
        cancelButton->setIcon(icon1);

        gridLayout->addWidget(cancelButton, 1, 1, 1, 1);


        retranslateUi(TaskEdit);

        QMetaObject::connectSlotsByName(TaskEdit);
    } // setupUi

    void retranslateUi(QWidget *TaskEdit)
    {
        TaskEdit->setWindowTitle(QApplication::translate("TaskEdit", "Form", 0));
        namePixmap->setText(QString());
        nameLabel->setText(QApplication::translate("TaskEdit", "Name", 0));
        pointsLabel->setText(QApplication::translate("TaskEdit", "Points", 0));
#ifndef QT_NO_TOOLTIP
        pointsInteger->setToolTip(QApplication::translate("TaskEdit", "Points for completion", 0));
#endif // QT_NO_TOOLTIP
        multiPixmap->setText(QString());
        multiLabel->setText(QApplication::translate("TaskEdit", "Max multiplier", 0));
#ifndef QT_NO_TOOLTIP
        multiInteger->setToolTip(QApplication::translate("TaskEdit", "Maximum point multiplier", 0));
#endif // QT_NO_TOOLTIP
        typeLabel->setText(QApplication::translate("TaskEdit", "Type", 0));
#ifndef QT_NO_TOOLTIP
        typeCombo->setToolTip(QApplication::translate("TaskEdit", "Task type (either checkbox or a multiplier task)", 0));
#endif // QT_NO_TOOLTIP
        stylePixmap->setText(QString());
        styleLabel->setText(QApplication::translate("TaskEdit", "Style", 0));
#ifndef QT_NO_TOOLTIP
        styleCombo->setToolTip(QApplication::translate("TaskEdit", "Style to hilight special tasks", 0));
#endif // QT_NO_TOOLTIP
        descPixmap->setText(QString());
        descLabel->setText(QApplication::translate("TaskEdit", "Description", 0));
        nameEdit->setPlaceholderText(QApplication::translate("TaskEdit", "Type task name here", 0));
        typePixmap->setText(QString());
        pointsPixmap->setText(QString());
        addButton->setText(QApplication::translate("TaskEdit", "Accept", 0));
        cancelButton->setText(QApplication::translate("TaskEdit", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class TaskEdit: public Ui_TaskEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TASKEDIT_H
