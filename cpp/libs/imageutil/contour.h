#ifndef _IMAGEUTIL_CONTOUR_H
#define _IMAGEUTIL_CONTOUR_H

// contour.h
// 4/5/2014 jichi

#include <QtGui/QImage>

//QImage contourImage(const QImage &src, const QColor &color, QPoint radius = 8, const QPoint &offset = QPoint(2,2), int alphaThreshold = 255);
QImage contourImage(const QImage &src, const QColor &color, QPoint radius, const QPoint &offset, int alphaThreshold = 255);

#endif // _IMAGEUTIL_CONTOUR_H
