// enginemanager.cc
// 4/26/2014 jichi

#include "engine/enginemanager.h"
#include "engine/enginehash.h"
#include "QxtCore/QxtJSON"
#include "qtjson/qtjson.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QHash>
#include <QtCore/QVariant>

/** Private class */

class EngineManagerPrivate
{
  QEventLoop *loop;
public:
  QHash<qint64, QString> texts, // {hash:text}
                         trs;   // {hash:tr}

  enum { TranslationTimeout = 5000 }; // wait for at most 5 seconds

  explicit EngineManagerPrivate(QObject *parent)
    : loop(new QEventLoop(parent))
  {
    QObject::connect(qApp, SIGNAL(aboutToQuit()), loop, SLOT(quit()));
  }

  // TODO: Add a timer to quit loop
  void block(int interval = 0)
  {
    if (!loop->isRunning())
      loop->exec();
  }

  void unblock()
  {
    if (loop->isRunning())
      loop->quit();
  }
};

/** Public class */

// - Construction -

EngineManager::EngineManager(QObject *parent) : Base(parent), d_(new D(this)) {}
EngineManager::~EngineManager() { delete d_; }

// - Actions -

void EngineManager::clearTranslation()  { d_->trs.clear(); }

void EngineManager::updateTranslation(const QString &json)
{
  QVariant data = QxtJSON::parse(json);
  if (!data.isNull()) {
    QVariantMap map = data.toMap();
    if (!map.isEmpty())
      for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        if (qint64 hash = it.key().toLongLong())
          d_->trs[hash] = it.value().toString();
  }
  d_->unblock();
}

void EngineManager::abortTranslation()
{ d_->unblock(); }

void EngineManager::updateText(const QString &text, qint64 hash)
{
  d_->texts[hash] = text;

  if (!d_->trs.contains(hash)) {
    QVariantHash map;
    map[QString::number(hash)] = text;
    QString json = QtJson::stringify(map);
    emit translationRequested(json);
  }
}

QString EngineManager::findTranslation(qint64 hash) const
{ return d_->trs.value(hash); }

QString EngineManager::waitForTranslation(qint64 hash) const
{
  // TODO: I didn't check the received translation hash
  d_->block(D::TranslationTimeout);
  return findTranslation(hash);
}

// EOF
