#ifndef _GRAPHICSEFFECT_IMAGEFILTERS_H
#define _GRAPHICSEFFECT_IMAGEFILTERS_H

// imagefilters.h
// 6/14/2012 jichi

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QPainter)
QT_FORWARD_DECLARE_CLASS(QImage)

// See: qt/src/gui/image/qpixmapfilter.cpp

void blurImage(QPainter *painter, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
void blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);

#endif // _GRAPHICSEFFECT_IMAGEFILTERS_H
