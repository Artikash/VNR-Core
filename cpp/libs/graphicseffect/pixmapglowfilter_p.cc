// pixmapglowfilter_p.cc
// 5/3/2012 jichi

#include "graphicseffect/pixmapglowfilter_p.h"
#include "graphicseffect/pixmapabstractblurfilter_p_p.h"
#include "graphicseffect/imagefilters_q.h"
#include "graphicseffect/pixmapfilter_q.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>

PixmapGlowFilter::PixmapGlowFilter(QObject *parent): Base(parent) {}
PixmapGlowFilter::~PixmapGlowFilter() {}

void PixmapGlowFilter::draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src) const
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

  // See: https://github.com/qtcn/tianchi/blob/master/src/gui/tcgloweffect.cpp
  qreal radius = d->radius;
  QImage tmp(px.size() + QSize(radius * 2, radius * 2), QImage::Format_ARGB32_Premultiplied);
  tmp.fill(0);
  QPainter tmpPainter(&tmp);
  tmpPainter.drawPixmap(d->offset, px);
  tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  tmpPainter.fillRect(tmp.rect(), d->color);
  tmpPainter.end();
  qt_blurImage(tmp, radius, true);

  // FIXME: This is very inefficient
  for (int i = 0; i < d->intensity; i++)
    p->drawImage(pos, tmp);

  p->drawPixmap(pos, px);
}

// EOF
