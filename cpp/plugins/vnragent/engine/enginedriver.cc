// enginedriver.cc
// 4/26/2014 jichi

#include "engine/enginedriver.h"
#include "engine/enginemanager.h"

/** Private class */

class EngineDriverPrivate
{
public:
  bool enabled;
  EngineManager *manager;

  explicit EngineDriverPrivate(QObject *parent)
    : enabled(true), manager(new EngineManager(parent)) {}
};

/** Public class */

EngineDriver::EngineDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(translationRequested(QString)), SIGNAL(translationRequested(QString)));
}

EngineDriver::~EngineDriver() { delete d_; }

void EngineDriver::updateTranslation(const QString &json) { d_->manager->updateTranslation(json); }
void EngineDriver::clearTranslation() { d_->manager->clearTranslation(); }

bool EngineDriver::isEnabled() const { return d_->enabled; }
void EngineDriver::setEnable(bool t) { d_->enabled = t; }

// EOF
