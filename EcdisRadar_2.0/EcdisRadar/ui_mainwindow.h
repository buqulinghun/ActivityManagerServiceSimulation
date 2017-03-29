/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Mar 22 16:24:29 2017
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *sailRecoder;
    QAction *alarmRecoder;
    QAction *chartSelect;
    QAction *displayAll;
    QAction *displayStand;
    QAction *displayBase;
    QAction *BRIGHT;
    QAction *WHITEBACK;
    QAction *BLACKBACK;
    QAction *DUSK;
    QAction *NIGHT;
    QAction *simplePoint;
    QAction *paperPoint;
    QAction *symbolArea;
    QAction *simpleArea;
    QAction *safeContour;
    QAction *chinese;
    QAction *english;
    QAction *layout;
    QAction *help;
    QAction *openAndClose;
    QAction *rangeAdd;
    QAction *rangeSec;
    QAction *offsetOps;
    QAction *gainOps;
    QAction *restrainOps;
    QAction *clutterOps;
    QAction *tuneMan;
    QAction *tuneAuto;
    QAction *action_2;
    QAction *showRadar;
    QAction *closeRadar;
    QAction *scaleMatch;
    QAction *green;
    QAction *yellow;
    QAction *red;
    QAction *highAlpha;
    QAction *middleAlpha;
    QAction *lowAlpha;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *menu_3;
    QMenu *menu_4;
    QMenu *menu_5;
    QMenu *RadarOps;
    QMenu *menu_6;
    QMenu *menu_7;
    QMenu *menu_9;
    QMenu *menu_10;
    QMenu *menu_8;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(600, 400);
        sailRecoder = new QAction(MainWindow);
        sailRecoder->setObjectName(QString::fromUtf8("sailRecoder"));
        alarmRecoder = new QAction(MainWindow);
        alarmRecoder->setObjectName(QString::fromUtf8("alarmRecoder"));
        chartSelect = new QAction(MainWindow);
        chartSelect->setObjectName(QString::fromUtf8("chartSelect"));
        displayAll = new QAction(MainWindow);
        displayAll->setObjectName(QString::fromUtf8("displayAll"));
        displayAll->setCheckable(true);
        displayStand = new QAction(MainWindow);
        displayStand->setObjectName(QString::fromUtf8("displayStand"));
        displayStand->setCheckable(true);
        displayBase = new QAction(MainWindow);
        displayBase->setObjectName(QString::fromUtf8("displayBase"));
        displayBase->setCheckable(true);
        BRIGHT = new QAction(MainWindow);
        BRIGHT->setObjectName(QString::fromUtf8("BRIGHT"));
        BRIGHT->setCheckable(true);
        WHITEBACK = new QAction(MainWindow);
        WHITEBACK->setObjectName(QString::fromUtf8("WHITEBACK"));
        WHITEBACK->setCheckable(true);
        BLACKBACK = new QAction(MainWindow);
        BLACKBACK->setObjectName(QString::fromUtf8("BLACKBACK"));
        BLACKBACK->setCheckable(true);
        DUSK = new QAction(MainWindow);
        DUSK->setObjectName(QString::fromUtf8("DUSK"));
        DUSK->setCheckable(true);
        NIGHT = new QAction(MainWindow);
        NIGHT->setObjectName(QString::fromUtf8("NIGHT"));
        NIGHT->setCheckable(true);
        simplePoint = new QAction(MainWindow);
        simplePoint->setObjectName(QString::fromUtf8("simplePoint"));
        simplePoint->setCheckable(true);
        paperPoint = new QAction(MainWindow);
        paperPoint->setObjectName(QString::fromUtf8("paperPoint"));
        paperPoint->setCheckable(true);
        symbolArea = new QAction(MainWindow);
        symbolArea->setObjectName(QString::fromUtf8("symbolArea"));
        symbolArea->setCheckable(true);
        simpleArea = new QAction(MainWindow);
        simpleArea->setObjectName(QString::fromUtf8("simpleArea"));
        simpleArea->setCheckable(true);
        safeContour = new QAction(MainWindow);
        safeContour->setObjectName(QString::fromUtf8("safeContour"));
        chinese = new QAction(MainWindow);
        chinese->setObjectName(QString::fromUtf8("chinese"));
        chinese->setCheckable(true);
        english = new QAction(MainWindow);
        english->setObjectName(QString::fromUtf8("english"));
        english->setCheckable(true);
        layout = new QAction(MainWindow);
        layout->setObjectName(QString::fromUtf8("layout"));
        help = new QAction(MainWindow);
        help->setObjectName(QString::fromUtf8("help"));
        openAndClose = new QAction(MainWindow);
        openAndClose->setObjectName(QString::fromUtf8("openAndClose"));
        openAndClose->setCheckable(true);
        rangeAdd = new QAction(MainWindow);
        rangeAdd->setObjectName(QString::fromUtf8("rangeAdd"));
        rangeSec = new QAction(MainWindow);
        rangeSec->setObjectName(QString::fromUtf8("rangeSec"));
        offsetOps = new QAction(MainWindow);
        offsetOps->setObjectName(QString::fromUtf8("offsetOps"));
        gainOps = new QAction(MainWindow);
        gainOps->setObjectName(QString::fromUtf8("gainOps"));
        restrainOps = new QAction(MainWindow);
        restrainOps->setObjectName(QString::fromUtf8("restrainOps"));
        clutterOps = new QAction(MainWindow);
        clutterOps->setObjectName(QString::fromUtf8("clutterOps"));
        tuneMan = new QAction(MainWindow);
        tuneMan->setObjectName(QString::fromUtf8("tuneMan"));
        tuneAuto = new QAction(MainWindow);
        tuneAuto->setObjectName(QString::fromUtf8("tuneAuto"));
        action_2 = new QAction(MainWindow);
        action_2->setObjectName(QString::fromUtf8("action_2"));
        showRadar = new QAction(MainWindow);
        showRadar->setObjectName(QString::fromUtf8("showRadar"));
        showRadar->setCheckable(true);
        closeRadar = new QAction(MainWindow);
        closeRadar->setObjectName(QString::fromUtf8("closeRadar"));
        closeRadar->setCheckable(true);
        scaleMatch = new QAction(MainWindow);
        scaleMatch->setObjectName(QString::fromUtf8("scaleMatch"));
        scaleMatch->setCheckable(true);
        green = new QAction(MainWindow);
        green->setObjectName(QString::fromUtf8("green"));
        green->setCheckable(true);
        yellow = new QAction(MainWindow);
        yellow->setObjectName(QString::fromUtf8("yellow"));
        yellow->setCheckable(true);
        red = new QAction(MainWindow);
        red->setObjectName(QString::fromUtf8("red"));
        red->setCheckable(true);
        highAlpha = new QAction(MainWindow);
        highAlpha->setObjectName(QString::fromUtf8("highAlpha"));
        highAlpha->setCheckable(true);
        middleAlpha = new QAction(MainWindow);
        middleAlpha->setObjectName(QString::fromUtf8("middleAlpha"));
        middleAlpha->setCheckable(true);
        lowAlpha = new QAction(MainWindow);
        lowAlpha->setObjectName(QString::fromUtf8("lowAlpha"));
        lowAlpha->setCheckable(true);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QString::fromUtf8("menu_2"));
        menu_3 = new QMenu(menuBar);
        menu_3->setObjectName(QString::fromUtf8("menu_3"));
        menu_4 = new QMenu(menuBar);
        menu_4->setObjectName(QString::fromUtf8("menu_4"));
        menu_5 = new QMenu(menuBar);
        menu_5->setObjectName(QString::fromUtf8("menu_5"));
        RadarOps = new QMenu(menuBar);
        RadarOps->setObjectName(QString::fromUtf8("RadarOps"));
        menu_6 = new QMenu(RadarOps);
        menu_6->setObjectName(QString::fromUtf8("menu_6"));
        menu_7 = new QMenu(menuBar);
        menu_7->setObjectName(QString::fromUtf8("menu_7"));
        menu_9 = new QMenu(menu_7);
        menu_9->setObjectName(QString::fromUtf8("menu_9"));
        menu_10 = new QMenu(menu_7);
        menu_10->setObjectName(QString::fromUtf8("menu_10"));
        menu_8 = new QMenu(menuBar);
        menu_8->setObjectName(QString::fromUtf8("menu_8"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menuBar->addAction(menu_3->menuAction());
        menuBar->addAction(menu_4->menuAction());
        menuBar->addAction(menu_7->menuAction());
        menuBar->addAction(RadarOps->menuAction());
        menuBar->addAction(menu_8->menuAction());
        menuBar->addAction(menu_5->menuAction());
        menu->addAction(chartSelect);
        menu_2->addAction(displayAll);
        menu_2->addAction(displayStand);
        menu_2->addAction(displayBase);
        menu_2->addSeparator();
        menu_2->addAction(BRIGHT);
        menu_2->addAction(WHITEBACK);
        menu_2->addAction(BLACKBACK);
        menu_2->addAction(DUSK);
        menu_2->addAction(NIGHT);
        menu_2->addSeparator();
        menu_2->addAction(simplePoint);
        menu_2->addAction(paperPoint);
        menu_2->addSeparator();
        menu_2->addAction(symbolArea);
        menu_2->addAction(simpleArea);
        menu_2->addSeparator();
        menu_2->addAction(safeContour);
        menu_2->addSeparator();
        menu_2->addAction(chinese);
        menu_2->addAction(english);
        menu_3->addAction(layout);
        menu_4->addAction(sailRecoder);
        menu_4->addAction(alarmRecoder);
        menu_5->addAction(help);
        RadarOps->addAction(openAndClose);
        RadarOps->addSeparator();
        RadarOps->addAction(rangeAdd);
        RadarOps->addAction(rangeSec);
        RadarOps->addSeparator();
        RadarOps->addAction(offsetOps);
        RadarOps->addSeparator();
        RadarOps->addAction(gainOps);
        RadarOps->addAction(restrainOps);
        RadarOps->addAction(clutterOps);
        RadarOps->addSeparator();
        RadarOps->addAction(menu_6->menuAction());
        RadarOps->addSeparator();
        RadarOps->addAction(action_2);
        menu_6->addAction(tuneMan);
        menu_6->addAction(tuneAuto);
        menu_7->addAction(showRadar);
        menu_7->addAction(closeRadar);
        menu_7->addSeparator();
        menu_7->addAction(menu_9->menuAction());
        menu_7->addSeparator();
        menu_7->addAction(menu_10->menuAction());
        menu_9->addAction(green);
        menu_9->addAction(yellow);
        menu_9->addAction(red);
        menu_10->addAction(highAlpha);
        menu_10->addAction(middleAlpha);
        menu_10->addAction(lowAlpha);
        menu_8->addAction(scaleMatch);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        sailRecoder->setText(QApplication::translate("MainWindow", "\350\210\252\350\241\214\350\256\260\345\275\225", 0, QApplication::UnicodeUTF8));
        alarmRecoder->setText(QApplication::translate("MainWindow", "\346\212\245\350\255\246\350\256\260\345\275\225", 0, QApplication::UnicodeUTF8));
        chartSelect->setText(QApplication::translate("MainWindow", "\346\265\267\345\233\276\351\200\211\346\213\251", 0, QApplication::UnicodeUTF8));
        displayAll->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272\345\205\250\351\203\250\344\277\241\346\201\257", 0, QApplication::UnicodeUTF8));
        displayStand->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272\346\240\207\345\207\206\344\277\241\346\201\257", 0, QApplication::UnicodeUTF8));
        displayBase->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272\345\237\272\346\234\254\344\277\241\346\201\257", 0, QApplication::UnicodeUTF8));
        BRIGHT->setText(QApplication::translate("MainWindow", "DAY BRIGHT", 0, QApplication::UnicodeUTF8));
        WHITEBACK->setText(QApplication::translate("MainWindow", "DAY WHITEBACK", 0, QApplication::UnicodeUTF8));
        BLACKBACK->setText(QApplication::translate("MainWindow", "DAY BLACKBACK", 0, QApplication::UnicodeUTF8));
        DUSK->setText(QApplication::translate("MainWindow", "DAY DUSK", 0, QApplication::UnicodeUTF8));
        NIGHT->setText(QApplication::translate("MainWindow", "NIGHT", 0, QApplication::UnicodeUTF8));
        simplePoint->setText(QApplication::translate("MainWindow", "\347\256\200\345\215\225\347\254\246\345\217\267", 0, QApplication::UnicodeUTF8));
        paperPoint->setText(QApplication::translate("MainWindow", "\344\274\240\347\273\237\347\272\270\346\265\267\345\233\276\347\254\246\345\217\267", 0, QApplication::UnicodeUTF8));
        symbolArea->setText(QApplication::translate("MainWindow", "\347\254\246\345\217\267\345\214\226\345\214\272\345\237\237\350\276\271", 0, QApplication::UnicodeUTF8));
        simpleArea->setText(QApplication::translate("MainWindow", "\347\256\200\345\215\225\345\214\226\345\214\272\345\237\237\350\276\271", 0, QApplication::UnicodeUTF8));
        safeContour->setText(QApplication::translate("MainWindow", "\345\256\211\345\205\250\347\272\277\350\256\276\345\256\232", 0, QApplication::UnicodeUTF8));
        chinese->setText(QApplication::translate("MainWindow", "\344\270\255\346\226\207\346\230\276\347\244\272", 0, QApplication::UnicodeUTF8));
        english->setText(QApplication::translate("MainWindow", "\350\213\261\346\226\207\346\230\276\347\244\272", 0, QApplication::UnicodeUTF8));
        layout->setText(QApplication::translate("MainWindow", "\345\233\276\345\261\202\347\256\241\347\220\206", 0, QApplication::UnicodeUTF8));
        help->setText(QApplication::translate("MainWindow", "\345\205\263\344\272\216\346\265\267\345\233\276\346\230\276\347\244\272\345\231\250", 0, QApplication::UnicodeUTF8));
        openAndClose->setText(QApplication::translate("MainWindow", "\346\211\223\345\274\200/\345\205\263\351\227\255", 0, QApplication::UnicodeUTF8));
        rangeAdd->setText(QApplication::translate("MainWindow", "\345\242\236\345\212\240\351\207\217\347\250\213", 0, QApplication::UnicodeUTF8));
        rangeSec->setText(QApplication::translate("MainWindow", "\345\207\217\345\260\221\351\207\217\347\250\213", 0, QApplication::UnicodeUTF8));
        offsetOps->setText(QApplication::translate("MainWindow", "\345\201\217\345\277\203\346\223\215\344\275\234", 0, QApplication::UnicodeUTF8));
        gainOps->setText(QApplication::translate("MainWindow", "\345\242\236\347\233\212\350\260\203\346\225\264", 0, QApplication::UnicodeUTF8));
        restrainOps->setText(QApplication::translate("MainWindow", "\351\233\250\351\233\252\350\260\203\346\225\264", 0, QApplication::UnicodeUTF8));
        clutterOps->setText(QApplication::translate("MainWindow", "\346\265\267\346\265\252\350\260\203\346\225\264", 0, QApplication::UnicodeUTF8));
        tuneMan->setText(QApplication::translate("MainWindow", "\346\211\213\345\212\250\350\260\203\350\260\220", 0, QApplication::UnicodeUTF8));
        tuneAuto->setText(QApplication::translate("MainWindow", "\350\207\252\345\212\250\350\260\203\350\260\220", 0, QApplication::UnicodeUTF8));
        action_2->setText(QApplication::translate("MainWindow", "\350\267\235\347\246\273\350\260\203\346\225\264", 0, QApplication::UnicodeUTF8));
        showRadar->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272\345\233\276\345\203\217", 0, QApplication::UnicodeUTF8));
        closeRadar->setText(QApplication::translate("MainWindow", "\345\205\263\351\227\255\345\233\276\345\203\217", 0, QApplication::UnicodeUTF8));
        scaleMatch->setText(QApplication::translate("MainWindow", "\346\257\224\344\276\213\345\260\272\345\214\271\351\205\215", 0, QApplication::UnicodeUTF8));
        green->setText(QApplication::translate("MainWindow", "\347\273\277\350\211\262", 0, QApplication::UnicodeUTF8));
        yellow->setText(QApplication::translate("MainWindow", "\351\273\204\350\211\262", 0, QApplication::UnicodeUTF8));
        red->setText(QApplication::translate("MainWindow", "\347\272\242\350\211\262", 0, QApplication::UnicodeUTF8));
        highAlpha->setText(QApplication::translate("MainWindow", "\344\275\216\351\200\217\346\230\216\345\272\246", 0, QApplication::UnicodeUTF8));
        middleAlpha->setText(QApplication::translate("MainWindow", "\344\270\255\351\200\217\346\230\216\345\272\246", 0, QApplication::UnicodeUTF8));
        lowAlpha->setText(QApplication::translate("MainWindow", "\351\253\230\351\200\217\346\230\216\345\272\246", 0, QApplication::UnicodeUTF8));
        menu->setTitle(QApplication::translate("MainWindow", "\345\233\276\345\272\223\347\256\241\347\220\206", 0, QApplication::UnicodeUTF8));
        menu_2->setTitle(QApplication::translate("MainWindow", "\346\230\276\347\244\272\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        menu_3->setTitle(QApplication::translate("MainWindow", "\345\233\276\345\261\202\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        menu_4->setTitle(QApplication::translate("MainWindow", "\350\256\260\345\275\225\346\237\245\350\257\242", 0, QApplication::UnicodeUTF8));
        menu_5->setTitle(QApplication::translate("MainWindow", "\345\270\256\345\212\251", 0, QApplication::UnicodeUTF8));
        RadarOps->setTitle(QApplication::translate("MainWindow", "\351\233\267\350\276\276\346\223\215\344\275\234", 0, QApplication::UnicodeUTF8));
        menu_6->setTitle(QApplication::translate("MainWindow", "\350\260\203\350\260\220\346\223\215\344\275\234", 0, QApplication::UnicodeUTF8));
        menu_7->setTitle(QApplication::translate("MainWindow", "\351\233\267\350\276\276\345\233\276\345\203\217", 0, QApplication::UnicodeUTF8));
        menu_9->setTitle(QApplication::translate("MainWindow", "\351\242\234\350\211\262\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        menu_10->setTitle(QApplication::translate("MainWindow", "\351\200\217\346\230\216\345\272\246\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        menu_8->setTitle(QApplication::translate("MainWindow", "\350\236\215\345\220\210\346\230\276\347\244\272\346\223\215\344\275\234", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
