// enginedriver.cc
// 4/26/2014 jichi

#include "growl.h"
#include "engine/enginedriver.h"
#include "engine/enginemanager.h"
#include "model/engine.h"
#include "qtjson/qtjson.h"
#include "QxtCore/QxtJSON"
#include <QtCore/QEventLoop>
#include <QtCore/QHash>
#include <QtCore/QVariant>

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
    if (p->inject())
      growl::notify(tr("Recognize game engine: %1").arg(p->name()));

  ::instance_ = this;
}

EngineDriver::~EngineDriver()
{
  ::instance_ = nullptr;
  delete d_;
}

void EngineDriver::updateTranslation(const QString &json) { d_->manager->updateTranslation(json); }
void EngineDriver::clearTranslation() { d_->manager->clearTranslation(); }

bool EngineDriver::isEnabled() const { return d_->enabled; }
void EngineDriver::setEnable(bool t) { d_->enabled = t; }

// Translate

QString EngineDriver::translate(const QString &text, qint64 hash)
{
  if (!d_->enabled)
    return QString();

  {
    QVariantHash map;
    map[QString::number(hash)] = text;
    QString json = QtJson::stringify(map);
    emit translationRequested(json);
  }

  QEventLoop loop;
  for sig in signals:
    sig.connect(loop.quit, type)

  # Make sure the eventloop quit before closing
  if autoQuit:
    QtCore.QCoreApplication.instance().aboutToQuit.connect(loop.quit)


  {
    QVariant data = QxtJSON::parse(json);
    if (data.isNull())
      return QString();
    QVariantMap map = data.toMap();
    if (map.isEmpty())
      return QString();

    auto it = map.constBegin();
    //qint64 hash = it.key().toLongLong();
    QString t = it.value().toString();
    return t;
  }
}

// EOF
