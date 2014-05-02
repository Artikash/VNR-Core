// settings.cc
// 5/1/2014 jichi

#include "driver/settings.h"
#include "ui/uidriver.h"

/** Private class */

class SettingsPrivate
{
public:
  bool uiTranslationEnabled;
  QString language;

  SettingsPrivate()
    : uiTranslationEnabled(false)
  {}
};

/** Public class */

//static Settings *instance_;
//Settings *Settings::instance() { return ::instance_; }

Settings::Settings(QObject *parent)
  : Base(parent), d_(new D)
{
  //instance_ = this;
}

Settings::~Settings()
{
  //instance_ = nullptr;
  delete d_;
}

#define DEFINE_PROPERTY(property, getter, setter, rettype, argtype) \
  rettype Settings::getter() \
  { return d_->property; } \
  void Settings::getter(argtype value)  \
  { if (d_->property != value) { d_->property = value; emit property##Changed(value); } }

DEFINE_PROPERTY(language, language, setLanguage, QString, const QString &)
DEFINE_PROPERTY(uiTranslationEnabled, isUiTranslationEnabled, setUiTranslationEnabled, bool, bool)

// EOF
