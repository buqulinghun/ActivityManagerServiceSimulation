/********************************************************************************
** Form generated from reading UI file 'factorDialog.ui'
**
** Created: Wed Mar 22 16:24:29 2017
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FACTORDIALOG_H
#define UI_FACTORDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_factorDialog
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *factorEdit;
    QPushButton *okButton;

    void setupUi(QDialog *factorDialog)
    {
        if (factorDialog->objectName().isEmpty())
            factorDialog->setObjectName(QString::fromUtf8("factorDialog"));
        factorDialog->resize(400, 82);
        gridLayout = new QGridLayout(factorDialog);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(factorDialog);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        factorEdit = new QLineEdit(factorDialog);
        factorEdit->setObjectName(QString::fromUtf8("factorEdit"));

        horizontalLayout->addWidget(factorEdit);

        okButton = new QPushButton(factorDialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);


        retranslateUi(factorDialog);

        QMetaObject::connectSlotsByName(factorDialog);
    } // setupUi

    void retranslateUi(QDialog *factorDialog)
    {
        factorDialog->setWindowTitle(QApplication::translate("factorDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("factorDialog", "\345\256\211\345\205\250\347\272\277\350\256\276\345\256\232\357\274\232", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("factorDialog", "\347\241\256\350\256\244", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class factorDialog: public Ui_factorDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FACTORDIALOG_H
