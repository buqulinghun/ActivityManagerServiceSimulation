#include "boatinfo.h"


#include <QStringList>


#ifndef TR
#define TR QString
#endif


QStringList localUpmodeText;
QStringList localMovingText;
QStringList localOffsetText;
QStringList localUnitsText;
QStringList localOnOffText;
QStringList localShowHideText;
QStringList localFullHalfAutoText;
QStringList localEBLReferenceText;

/*获取字符串*/
QString upmodeDisplayText(quint8 index)
{   return localUpmodeText.value(index);    }
QString movingDisplayText(quint8 index)
{   return localMovingText.value(index);    }
QString offsetDisplayText(quint8 index)
{   return localOffsetText.value(index);    }
QString unitsDisplayText(quint8 index)
{   return localUnitsText.value(index);    }
QString vrmText(quint8 index)
{
        return "";
}
QString eblText(quint8 index)
{
        return "";
}



/*初始化显示文本*/
void retranslateGlobalText()
{
    localUpmodeText.clear();
    localMovingText.clear();
    localOffsetText.clear();
    localUnitsText.clear();
    localOnOffText.clear();
    localShowHideText.clear();
    localFullHalfAutoText.clear();
    localEBLReferenceText.clear();

    localUpmodeText << TR("N-UP") << TR("H-UP") << TR("C-UP");
    localMovingText << TR("TM") << TR("RM");
    localOffsetText << TR("0") << TR("1/3") << TR("2/3");
    localUnitsText << TR("km") << TR("nv") << TR("mi");
    localOnOffText << TR("Off") << TR("On");
    localShowHideText << TR("Hide") << TR("Show");
    localFullHalfAutoText << TR("FullAuto") << TR("HalfAuto");
    localEBLReferenceText << TR("RealAzi") << TR("DelaAzi");
}

