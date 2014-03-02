// qmleffectsplugin.cc
// 11/12/2012 jichi
#include "qmleffectsplugin/qmleffectsplugin.h"
#include "graphicseffect/graphicsgloweffect.h"
#include "graphicseffect/graphicsgloweffect2.h"
#include "graphicseffect/graphicstextshadoweffect.h"
#include <QtDeclarative/qdeclarative.h>

QmlEffectsPlugin::QmlEffectsPlugin(QObject *parent)
  : Base(parent) {}

void QmlEffectsPlugin::registerTypes(const char *uri)
{
  qmlRegisterType<GraphicsTextShadowEffect>(uri, 1, 0, "TextShadow");
  qmlRegisterType<GraphicsGlowEffect>(uri, 1, 0, "Glow");
  qmlRegisterType<GraphicsGlowEffect2>(uri, 1, 0, "Glow2");
}

Q_EXPORT_PLUGIN2(qmleffectsplugin, QmlEffectsPlugin);

// EOF
