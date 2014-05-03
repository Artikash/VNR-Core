// embeddriver.cc
// 4/26/2014 jichi

#include "growl.h"
#include "embed/embeddriver.h"
#include "embed/embedmanager.h"
#include "engine/engine.h"

/** Private class */

class EmbedDriverPrivate
{
public:
  bool enabled;
  EmbedManager *manager;

  explicit EmbedDriverPrivate(QObject *parent)
    : enabled(true), manager(new EmbedManager(parent)) {}
};

/** Public class */

//static EmbedDriver *instance_;
//EmbedDriver *EmbedDriver::instance() { return instance_; }

EmbedDriver::EmbedDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(d_->manager, SIGNAL(textReceived(QString,qint64,int,bool)), SIGNAL(textReceived(QString,qint64,int,bool)));

  if (auto p = AbstractEngine::instance())
    //p->setParent(this);
    if (p->inject()) {
      //d_->engine = p;
      //connect(d_->engine, SIGNAL(textReceived(QString,qint64,int,void*)), d_->manager, SLOT(addText(QString,qint64,int,void*)));
      // FIXME: Only one instance can be send at a time?!
      growl::notify(tr("Recognize game engine: %1").arg(p->name()));
    }

  //::instance_ = this;
}

EmbedDriver::~EmbedDriver()
{
  //::instance_ = nullptr;
  delete d_;
}

bool EmbedDriver::isEnabled() const { return d_->enabled; }
void EmbedDriver::setEnable(bool t) { d_->enabled = t; }

void EmbedDriver::clearTranslation()  { d_->manager->clearTranslation(); }

void EmbedDriver::updateTranslation(const QString &text, qint64 hash, int role)
{ d_->manager->updateTranslation(text, hash, role); }


// EOF
