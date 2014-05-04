// settings.cc
// 5/1/2014 jichi

#include "driver/settings.h"

/** Private class */

class SettingsPrivate
{
public:
  bool enabled,
       engineEnabled,
       windowTranslationEnabled;
  //QString language;

  SettingsPrivate()
    : enabled(false),
      engineEnabled(false),
      windowTranslationEnabled(false)
  {}
};

/** Public class */

static Settings *instance_;
Settings *Settings::instance() { return ::instance_; }

Settings::Settings(QObject *parent)
  : Base(parent), d_(new D)
{
  instance_ = this;
}

Settings::~Settings()
{
  instance_ = nullptr;
  delete d_;
}

#define DEFINE_PROPERTY(property, getter, setter, rettype, argtype) \
  rettype Settings::getter() const \
  { return d_->property; } \
  void Settings::setter(argtype value)  \
  { if (d_->property != value) { d_->property = value; emit property##Changed(value); } }

DEFINE_PROPERTY(enabled, isEnabled, setEnabled, bool, bool)
DEFINE_PROPERTY(engineEnabled, isEngineEnabled, setEngineEnabled, bool, bool)
DEFINE_PROPERTY(windowTranslationEnabled, isWindowTranslationEnabled, setWindowTranslationEnabled, bool, bool)
//DEFINE_PROPERTY(language, language, setLanguage, QString, const QString &)

// Marshal

#include "QxtCore/QxtJSON"
#include <QtCore/QVariantMap>
#include "util/msghandler.h"

void Settings::load(const QString &json)
{
  enum {
    H_DEBUG = 6994359 // "debug"
    , H_ENGINE_ENABLE = 207122565   // "engine.enable"
    , H_WINDOW_ENABLE = 147657733   // "window.enable"
  };
  QVariant data = QxtJSON::parse(json);
  if (data.isNull())
    return;
  QVariantMap map = data.toMap();
  if (map.isEmpty())
    return;

  for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
    QString value = it.value().toString();
    bool bValue = value == "true";
    switch (qHash(it.key())) {
    case H_DEBUG: if (bValue) Util::installDebugMsgHandler(); break;
    case H_ENGINE_ENABLE: setEngineEnabled(bValue); break;
    case H_WINDOW_ENABLE: setWindowTranslationEnabled(bValue); break;
    default: ;
    }
  }
}

// EOF
