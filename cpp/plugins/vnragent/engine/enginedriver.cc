// enginedriver.cc
// 4/26/2014 jichi

#include "growl.h"
#include "engine/enginedriver.h"
#include "engine/enginemanager.h"
#include "model/engine.h"

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

static EngineDriver *instance_;
EngineDriver *EngineDriver::instance() { return instance_; }

EngineDriver::EngineDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(translationRequested(QString)), SIGNAL(translationRequested(QString)));

  if (auto p = AbstractEngine::instance())
    if (p->inject()) {
      // FIXME: Only one instance can be send at a time?!
      //growl::notify(tr("Recognize game engine: %1").arg(p->name()));
    }

  ::instance_ = this;
}

EngineDriver::~EngineDriver()
{
  ::instance_ = nullptr;
  delete d_;
}

void EngineDriver::updateTranslation(const QString &json) { d_->manager->updateTranslation(json); }
void EngineDriver::clearTranslation() { d_->manager->clearTranslation(); }
void EngineDriver::abortTranslation() { d_->manager->abortTranslation(); }

bool EngineDriver::isEnabled() const { return d_->enabled; }
void EngineDriver::setEnable(bool t) { d_->enabled = t; }

void EngineDriver::quit() { d_->manager->quit(); }

// Translate

QString EngineDriver::translate(const QString &text, qint64 hash, bool block)
{
  if (!d_->enabled)
    return QString();

  d_->manager->updateText(text, hash);
  QString ret = d_->manager->findTranslation(hash);
  block = false;
  if (ret.isEmpty() && block)
    ret = d_->manager->waitForTranslation(hash);
  return ret;
}

// EOF
