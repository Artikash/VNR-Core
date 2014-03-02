#ifndef _GRAPHICSEFFECT_GRAPHICSTEXTSHADOWEFFECT_H
#define _GRAPHICSEFFECT_GRAPHICSTEXTSHADOWEFFECT_H

// graphicstextshadoweffect.h
// 5/3/2012 jichi
// See: qt/src/gui/effects/qgraphicseffect.h

#include "graphicseffect/graphicsabstractblureffect_p.h"

class GraphicsTextShadowEffect: public GraphicsAbstractBlurEffect
{
  Q_OBJECT
  Q_DISABLE_COPY(GraphicsTextShadowEffect)
  typedef GraphicsTextShadowEffect Self;
  typedef GraphicsAbstractBlurEffect Base;

public:
  explicit GraphicsTextShadowEffect(QObject *parent = nullptr);
  ~GraphicsTextShadowEffect();
};

#endif // _GRAPHICSEFFECT_GRAPHICSTEXTSHADOWEFFECT_H
