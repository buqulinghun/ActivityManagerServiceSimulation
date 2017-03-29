/********************************************************************************
** Form generated from reading UI file 'adjustDialog.ui'
**
** Created: Wed Mar 22 16:24:29 2017
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADJUSTDIALOG_H
#define UI_ADJUSTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_adjustDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *adjustLabel;
    QHBoxLayout *horizontalLayout;
    QToolButton *decButton;
    QProgressBar *numBar;
    QToolButton *addButton;
    QToolButton *enterButton;

    void setupUi(QDialog *adjustDialog)
    {
        if (adjustDialog->objectName().isEmpty())
            adjustDialog->setObjectName(QString::fromUtf8("adjustDialog"));
        adjustDialog->resize(482, 86);
        verticalLayout = new QVBoxLayout(adjustDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        adjustLabel = new QLabel(adjustDialog);
        adjustLabel->setObjectName(QString::fromUtf8("adjustLabel"));
        adjustLabel->setMinimumSize(QSize(0, 30));
        adjustLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(adjustLabel);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        decButton = new QToolButton(adjustDialog);
        decButton->setObjectName(QString::fromUtf8("decButton"));
        decButton->setMinimumSize(QSize(50, 30));

        horizontalLayout->addWidget(decButton);

        numBar = new QProgressBar(adjustDialog);
        numBar->setObjectName(QString::fromUtf8("numBar"));
        numBar->setMinimumSize(QSize(200, 30));
        numBar->setValue(24);
        numBar->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(numBar);

        addButton = new QToolButton(adjustDialog);
        addButton->setObjectName(QString::fromUtf8("addButton"));
        addButton->setMinimumSize(QSize(50, 30));

        horizontalLayout->addWidget(addButton);

        enterButton = new QToolButton(adjustDialog);
        enterButton->setObjectName(QString::fromUtf8("enterButton"));
        enterButton->setMinimumSize(QSize(50, 30));

        horizontalLayout->addWidget(enterButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(adjustDialog);

        QMetaObject::connectSlotsByName(adjustDialog);
    } // setupUi

    void retranslateUi(QDialog *adjustDialog)
    {
        adjustDialog->setWindowTitle(QApplication::translate("adjustDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        adjustLabel->setText(QApplication::translate("adjustDialog", "\350\260\203\350\212\202\351\241\271\347\233\256", 0, QApplication::UnicodeUTF8));
        decButton->setText(QApplication::translate("adjustDialog", "\345\207\217\345\260\221 -", 0, QApplication::UnicodeUTF8));
        numBar->setFormat(QApplication::translate("adjustDialog", "%v", 0, QApplication::UnicodeUTF8));
        addButton->setText(QApplication::translate("adjustDialog", "\345\242\236\345\212\240 +", 0, QApplication::UnicodeUTF8));
        enterButton->setText(QApplication::translate("adjustDialog", "\347\241\256\350\256\244", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class adjustDialog: public Ui_adjustDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADJUSTDIALOG_H
