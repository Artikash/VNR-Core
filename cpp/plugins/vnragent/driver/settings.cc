// settings.cc
// 5/1/2014 jichi

#include "driver/settings.h"

/** Private class */

class SettingsPrivate
{
public:
  bool windowTranslationEnabled
     , windowTranscodingEnabled
     , windowTextVisible
     , embeddedScenarioVisible
     , embeddedScenarioTranslationEnabled
     , embeddedNameVisible
     , embeddedNameTranslationEnabled
     , embeddedOtherVisible
     , embeddedOtherTranslationEnabled;

  SettingsPrivate()
     : windowTranslationEnabled(false)
     , windowTranscodingEnabled(false)
     , windowTextVisible(false)
     , embeddedScenarioVisible(false)
     , embeddedScenarioTranslationEnabled(false)
     , embeddedNameVisible(false)
     , embeddedNameTranslationEnabled(false)
     , embeddedOtherVisible(false)
     , embeddedOtherTranslationEnabled(false)
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

#define DEFINE_STRING_PROPERTY(property, getter, setter)    DEFINE_PROPERTY(property, setter, getter, QString, const QString &)
#define DEFINE_BOOL_PROPERTY(property, getter, setter)      DEFINE_PROPERTY(property, setter, getter, bool, bool)

DEFINE_BOOL_PROPERTY(windowTranslationEnabled, isWindowTranslationEnabled, setWindowTranslationEnabled)

DEFINE_BOOL_PROPERTY(windowTranslationEnabled, isWindowTranslationEnabled, setWindowTranslationEnabled)
DEFINE_BOOL_PROPERTY(windowTranscodingEnabled, isWindowTranscodingEnabled, setWindowTranscodingEnabled)
DEFINE_BOOL_PROPERTY(windowTextVisible, isWindowTextVisible, setWindowTextVisible)
DEFINE_BOOL_PROPERTY(embeddedScenarioVisible, isEmbeddedScenarioVisible, setEmbeddedScenarioVisible)
DEFINE_BOOL_PROPERTY(embeddedScenarioTranslationEnabled, isEmbeddedScenarioTranslationEnabled, setEmbeddedScenarioTranslationEnabled)
DEFINE_BOOL_PROPERTY(embeddedNameVisible, isEmbeddedNameVisible, setEmbeddedNameVisible)
DEFINE_BOOL_PROPERTY(embeddedNameTranslationEnabled, isEmbeddedNameTranslationEnabled, setEmbeddedNameTranslationEnabled)
DEFINE_BOOL_PROPERTY(embeddedOtherVisible, isEmbeddedOtherVisible, setEmbeddedOtherVisible)
DEFINE_BOOL_PROPERTY(embeddedOtherTranslationEnabled, isEmbeddedOtherTranslationEnabled, setEmbeddedOtherTranslationEnabled)

// Marshal

#include "QxtCore/QxtJSON"
#include <QtCore/QVariantMap>
#include "util/msghandler.h"

void Settings::load(const QString &json)
{
  enum {
    H_DEBUG = 6994359 // "debug"
    //, H_ENGINE_ENABLE = 207122565   // "engine.enable"
    //, H_WINDOW_ENABLE = 147657733   // "window.enable"
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
    //case H_ENGINE_ENABLE: setEngineEnabled(bValue); break;
    //case H_WINDOW_ENABLE: setWindowTranslationEnabled(bValue); break;
    default: ;
    }
  }
}

// EOF
