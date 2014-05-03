// windowdriver.cc
// 2/1/2013 jichi

#include "window/windowdriver.h"
#include "window/windowdriver_p.h"
#include "window/windowmanager.h"

WindowDriver::WindowDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(textsChanged(QString)), SIGNAL(translationRequested(QString)));
}

void WindowDriver::updateTranslation(const QString &json) { d_->manager->updateTranslation(json); }
void WindowDriver::clearTranslation() { d_->manager->clearTranslation(); }

bool WindowDriver::isEnabled() const { return d_->enabled; }
void WindowDriver::setEnable(bool t) { d_->enabled = t; }

// EOF
