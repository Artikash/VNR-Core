// uidriver.cc
// 2/1/2013 jichi

#include "global.h"
#include "ui/uidriver.h"
#include "ui/uidriver_p.h"
#include "ui/uimanager.h"

UiDriver::UiDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(textsChanged(QString)), SIGNAL(translationRequested(QString)));
}

~UiDriver() { delete d_; }

void UiDriver::updateTranslation(QString json) { d_->manager->updateTranslationData(json); }
void UiDriver::clearTranslation() { d_->manager->clearTranslation(); }

bool UiDriver::isEnabled() const { return d_->enabled; }
void UiDriver::setEnable(bool t) { d_->enabled = t; }

// EOF
