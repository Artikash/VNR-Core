// pixmaptextshadowfilter_p.cc
// 5/3/2012 jichi
// See: gui/image/pixmapfilter_p.cpp

#include "graphicseffect/pixmaptextshadowfilter_p.h"
#include "graphicseffect/pixmapabstractblurfilter_p_p.h"
#include "graphicseffect/imagefilters.h"
#include "graphicseffect/pixmapfilter_q.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>

PixmapTextShadowFilter::PixmapTextShadowFilter(QObject *parent): Base(parent) {}
PixmapTextShadowFilter::~PixmapTextShadowFilter() {}

void PixmapTextShadowFilter::draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src) const
{
  if (px.isNull())
    return;
  Q_D(const PixmapAbstractBlurFilter);
  if (QPaintEngine *e = p->paintEngine())
    if (e->isExtended())
      if (QPixmapFilter *filter = static_cast<QPaintEngineEx *>(e)->pixmapFilter(type(), this))
        if (Self *that = dynamic_cast<Self *>(filter)) {
          that->setColor(d->color);
          that->setBlurIntensity(d->intensity);
          that->setBlurRadius(d->radius);
          that->setOffset(d->offset);
          that->draw(p, pos, px, src);
          return;
        }

  //QImage tmp(px.size() + QSize(d->radius * 2, d->radius * 2),
  QImage tmp(px.size(), QImage::Format_ARGB32_Premultiplied);
  tmp.fill(0);
  QPainter tmpPainter(&tmp);
  tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
  tmpPainter.drawPixmap(d->offset, px);
  tmpPainter.end();

  // blur the alpha channel
  QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
  blurred.fill(0);

  QPainter blurPainter(&blurred);
  for (int i = 0; i < d->intensity; i++)
    ::blurImage(&blurPainter, tmp, d->radius, false, true); // quality = false, alphaOnly = true
  blurPainter.end();

  // blacken the image...
  QPainter blackenPainter(&blurred);
  blackenPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  blackenPainter.fillRect(blurred.rect(), d->color);
  blackenPainter.end();

  // draw the blurred drop shadow...
  p->drawImage(pos, blurred);

  // Draw the actual pixmap...
  p->drawPixmap(pos, px, src);
}

// EOF
