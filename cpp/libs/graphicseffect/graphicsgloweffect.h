#ifndef _GRAPHICSEFFECT_GRAPHICSGLOWEFFECT_H
#define _GRAPHICSEFFECT_GRAPHICSGLOWEFFECT_H

// graphicsGloweffect.h
// 5/3/2012 jichi
// See: qt/src/gui/effects/qgraphicseffect.h

#include "graphicseffect/graphicsabstractblureffect_p.h"

class GraphicsGlowEffect: public GraphicsAbstractBlurEffect
{
  Q_OBJECT
  Q_DISABLE_COPY(GraphicsGlowEffect)
  typedef GraphicsGlowEffect Self;
  typedef GraphicsAbstractBlurEffect Base;

public:
  explicit GraphicsGlowEffect(QObject *parent = nullptr);
  ~GraphicsGlowEffect();
};

#endif // _GRAPHICSEFFECT_GRAPHICSGLOWEFFECT_H
