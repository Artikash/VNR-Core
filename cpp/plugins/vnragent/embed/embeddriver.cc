// embeddriver.cc
// 4/26/2014 jichi

#include "embed/embeddriver.h"
#include "embed/embedmanager.h"
#include "engine/engine.h"

/** Private class */

class EmbedDriverPrivate
{
public:
  bool enabled;
  EmbedManager *manager;
  AbstractEngine *engine;

  explicit EmbedDriverPrivate(QObject *parent)
    : enabled(true), manager(new EmbedManager(parent)), engine(nullptr) {}

  ~EmbedDriverPrivate() { if (engine) delete engine; }
};

/** Public class */

//static EmbedDriver *instance_;
//EmbedDriver *EmbedDriver::instance() { return instance_; }

EmbedDriver::EmbedDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(textReceived(QString,qint64,int,bool)), SIGNAL(textReceived(QString,qint64,int,bool)));
  connect(d_->manager, SIGNAL(textReceivedDelayed(QString,qint64,int,bool)), SIGNAL(textReceivedDelayed(QString,qint64,int,bool)));
}

EmbedDriver::~EmbedDriver()
{
  //::instance_ = nullptr;
  delete d_;
}

bool EmbedDriver::inject()
{
  if (d_->engine = AbstractEngine::instance())
    if (!d_->engine->inject())
      d_->engine = nullptr;

  sendEngineName();
  return d_->engine;
}

bool EmbedDriver::isEnabled() const { return d_->enabled; }
void EmbedDriver::setEnabled(bool t) { d_->enabled = t; }

void EmbedDriver::clearTranslation()  { d_->manager->clearTranslation(); }

void EmbedDriver::updateTranslation(const QString &text, qint64 hash, int role)
{ d_->manager->updateTranslation(text, hash, role); }

void EmbedDriver::unload()
{
  if (d_->engine)
    d_->engine->unload();
}

QString EmbedDriver::engineName() const
{ return d_->engine ? d_->engine->name() : QString(); }

void EmbedDriver::sendEngineName()
{
  emit engineNameChanged(engineName());
}

// EOF
