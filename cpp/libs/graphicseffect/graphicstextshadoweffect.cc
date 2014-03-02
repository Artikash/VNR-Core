// graphicstextshadoweffect.cc
// 5/3/2012 jichi
// See: gui/effects/qgraphicseffect.cpp
#include "graphicseffect/graphicstextshadoweffect.h"
#include "graphicseffect/pixmaptextshadowfilter_p.h"

// - Construction -

GraphicsTextShadowEffect::GraphicsTextShadowEffect(QObject *parent)
  : Base(new PixmapTextShadowFilter, parent) {}

GraphicsTextShadowEffect::~GraphicsTextShadowEffect() {}

// EOF
