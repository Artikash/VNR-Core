// enginemanager.cc
// 4/26/2014 jichi

#include "engine/enginemanager.h"
#include "engine/enginehash.h"
#include "QxtCore/QxtJSON"
#include "qtjson/qtjson.h"
#include <QtCore/QHash>
#include <QtCore/QVariant>

//#define DEBUG "EngineManager"
//#include "growl.h"

/** Private class */

class EngineManagerPrivate
{
  SK_CLASS(EngineManagerPrivate)
  SK_DISABLE_COPY(EngineManagerPrivate)
  //SK_DECLARE_PUBLIC(EngineManager)

public:
  QHash<qint64, QString> texts, // {hash:text}
                         trs;   // {hash:tr}
};

/** Public class */

// - Construction -

EngineManager::EngineManager(QObject *parent) : Base(parent), d_(new D) {}
EngineManager::~EngineManager() { delete d_; }

// - Actions -

void EngineManager::clearTranslation()  { d_->trs.clear(); }

void EngineManager::updateTranslation(const QString &json)
{
  QVariant data = QxtJSON::parse(json);
  if (data.isNull())
    return;
  QVariantMap map = data.toMap();
  if (map.isEmpty())
    return;

  for (auto it = map.constBegin(); it != map.constEnd(); ++it)
    if (qint64 hash = it.key().toLongLong())
      d_->trs[hash] = it.value().toString();
}

// EOF
