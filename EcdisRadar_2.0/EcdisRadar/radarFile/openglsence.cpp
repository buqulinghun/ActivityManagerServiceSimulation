/****************************************************************************
file name: openglsence.cpp
author: wang ming xiao
date: 2015/07/25
comments:  场景设置文件，在其中添加点迹航迹等item
***************************************************************************/
#include "openglsence.h"
#include "define.h"

#include <QGraphicsProxyWidget>   //用于在场景中添加widget部件



OpenGLSence::OpenGLSence(QObject *parent) :
    QGraphicsScene(parent)
{
   this->setSceneRect(0,0,MAINVIEW_RADIUS, MAINVIEW_RADIUS);

}



