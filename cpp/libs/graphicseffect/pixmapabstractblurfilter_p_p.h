#ifndef _GRAPHICSEFFECT_PIXMAPABSTRACTBLURFILTER_P_P_H
#define _GRAPHICSEFFECT_PIXMAPABSTRACTBLURFILTER_P_P_H

// pixmapabstractblurfilter_p_p.h
// 5/3/2012 jichi

#include "graphicseffect/pixmapfilter_q.h"

class PixmapAbstractBlurFilterPrivate : public QPixmapFilterPrivate
{
  Q_DECLARE_PUBLIC(PixmapAbstractBlurFilter)

public:
  int intensity;
  QPointF offset;
  QColor color;
  qreal radius;

  PixmapAbstractBlurFilterPrivate():
    intensity(1),
    //offset(0, 0), //offset(8, 8),
    //color(Qt::black),
    radius(1) {}
};

#endif // _GRAPHICSEFFECT_PIXMAPABSTRACTBLURFILTER_P_P_H
