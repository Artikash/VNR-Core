#ifndef _GRAPHICSEFFECT_PIXMAPTEXTSHADOWFILTER_P_H
#define _GRAPHICSEFFECT_PIXMAPTEXTSHADOWFILTER_P_H

// pixmaptextshadowfilter_p.h
// 5/3/2012 jichi
// See: gui/image/pixmapfilter_p.h

#include "graphicseffect/pixmapabstractblurfilter_p.h"

class PixmapTextShadowFilter : public PixmapAbstractBlurFilter
{
  Q_OBJECT
  Q_DISABLE_COPY(PixmapTextShadowFilter)
  typedef PixmapTextShadowFilter Self;
  typedef PixmapAbstractBlurFilter Base;

  PixmapAbstractBlurFilterPrivate *d_func() { return reinterpret_cast<PixmapAbstractBlurFilterPrivate *>(qGetPtrHelper(d_ptr)); } \
  const PixmapAbstractBlurFilterPrivate *d_func() const { return reinterpret_cast<const PixmapAbstractBlurFilterPrivate *>(qGetPtrHelper(d_ptr)); } \
public:
  explicit PixmapTextShadowFilter(QObject *parent = nullptr);
  ~PixmapTextShadowFilter();
  void draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src = QRectF()) const override;
};

#endif // _GRAPHICSEFFECT_PIXMAPTEXTSHADOWFILTER_P_H
