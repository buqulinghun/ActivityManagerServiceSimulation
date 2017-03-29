#ifndef TARGETTRANSFORM_H
#define TARGETTRANSFORM_H

#include "TargetManage_global.h"
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QList>

class CViewTransform;

class CTargetTransform
{
public:
    CTargetTransform()
    {
        for(int i=0;i<MAXVIEWCOUNT; i++)
            m_displayflag[i] = (1<<i);
    }

    // 设置坐标变换对象
    void setTransform(CViewTransform* transform, quint8 count)
    {
        m_transform = transform;
        m_transformCount = qMin(count, (quint8)MAXVIEWCOUNT);
    }

    //更新所有目标的屏幕坐标
    void updateScreenPoint(PLOTPOSITION& position)
    {
        position.displayFlag = 0;
        for(int i=0; i<m_transformCount; i++)
        {
            position.screen_point[i] = m_transform[i].square_to_screen(position.square_point);
            if(m_transform[i].screenEnvelope().contains(position.screen_point[i].toPoint()))
                position.displayFlag |= m_displayflag[i];
        }
    }

    void updateScreenPoint(PLOTPOSITION& position, quint8 idx)
    {
        position.screen_point[idx] = m_transform[idx].square_to_screen(position.square_point);
        if(m_transform[idx].screenEnvelope().contains(position.screen_point[idx].toPoint()))
            position.displayFlag |= m_displayflag[idx];
        else
            position.displayFlag &= (~m_displayflag[idx]);
    }

    //更新所有目标的直角坐标和屏幕坐标，距离量纲变化时调用
    void updateSquareAndScreenPoint(PLOTPOSITION& position)
    {
        position.displayFlag = 0;
        for(int i=0; i<m_transformCount; i++)
        {
            position.square_point = m_transform[i].latitude_to_square(position.latitude_point);
            position.rtheta_point = m_transform[i].square_to_rtheta(position.square_point);
            position.screen_point[i] = m_transform[i].square_to_screen(position.square_point);
            if(m_transform[i].screenEnvelope().contains(position.screen_point[i].toPoint()))
                position.displayFlag |= displayflag[i];
        }
    }

    //目标位置移动处理
    void move (PLOTPOSITION& position, const QPointF& sq, const QList<QPoint>& sc, quint8 index)
    {
        position.square_point += sq;
        for(int i=0,j=0; i<m_transformCount; i++)
        {
            if(index & displayflag[i])
            {
                position.screen_point[i] += sc[j];
                if(m_transform[i].screenEnvelope().contains(position.screen_point[i].toPoint()))
                    position.displayFlag |= displayflag[i];
                else
                    position.displayFlag &= (~displayflag[i]);
                j++;
            }
        }
    }

protected:
    quint32 m_displayflag[MAXVIEWCOUNT];
    // 坐标转换对象
    CViewTransform*   m_transform;
    quint8  m_transformCount;
};

#endif // TARGETTRANSFORM_H
