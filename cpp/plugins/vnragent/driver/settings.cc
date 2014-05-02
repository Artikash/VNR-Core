// settings.cc
// 5/1/2014 jichi

#include "driver/settings.h"
#include "ui/uidriver.h"

/** Private class */

class SettingsPrivate
{
public:
  QString language;
};

/** Public class */

static Settings *instance_;
Settings *Settings::instance(){ return ::instance_; }

Settings::Settings(QObject *parent)
  : Base(parent), d_(new D)
{ instance_ = this; }

Settings::~Settings()
{
  instance_ = nullptr;
  delete d_;
}

#define DEFINE_PROPERTY(getter, setter, rettype, argtype) \
  rettype Settings::getter() { return d_->getter; } \
  void Settings::getter(argtype value) { d_->getter = value; }

DEFINE_PROPERTY(language, setLanguage, QString, const QString &)

// EOF
