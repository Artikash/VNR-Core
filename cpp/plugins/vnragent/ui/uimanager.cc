// uimanager.cc
// 2/1/2013 jichi

#include "ui/uimanager.h"
#include "ui/uihash.h"
#include "sakurakit/skhash.h"
#include "QxtCore/QxtJSON"
#include "qtjson/qtjson.h"
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <unordered_set>

//#define DEBUG "uimanager"
//#include "growl.h"

/** Private class */

namespace { UiManager::TextEntry NULL_TEXT_ENTRY; }

class UiManagerPrivate
{
  SK_CLASS(UiManagerPrivate)
  SK_DISABLE_COPY(UiManagerPrivate)
  //SK_DECLARE_PUBLIC(UiManager)
  typedef UiManager Q;

  enum { RefreshInterval = 200 };
  QTimer *refreshTextsTimer_; // QTimer is not working
public:
  bool textsDirty;
  Q::TextEntryList entries;
  QHash<qint64, QString> trs;   // {hash : tr}

  std::unordered_set<qint64> h_texts; // hashes
  std::unordered_set<qint64> h_trs; // hashes

public:
  explicit UiManagerPrivate(Q *q) : textsDirty(false)
  {
    refreshTextsTimer_ = new QTimer(q);
    refreshTextsTimer_->setSingleShot(true);
    refreshTextsTimer_->setInterval(RefreshInterval);
    q->connect(refreshTextsTimer_, SIGNAL(timeout()), SLOT(refreshTexts()));
  }

  //~UiManagerPrivate()
  //{
  //  if (refreshTextsTimer_.isActive())
  //    refreshTextsTimer_.stop();
  //}

  void touchTexts()
  {
    textsDirty = true;
    //if (!refreshTextsTimer_->isActive())
    refreshTextsTimer_->start();
  }
};

/** Public class */

// - Construction -

static UiManager *instance_;
UiManager *UiManager::instance() { return instance_; }

UiManager::UiManager(QObject *parent)
  : Base(parent), d_(new D(this))
{
  ::instance_ = this;
}

UiManager::~UiManager()
{
  ::instance_ = nullptr;
  delete d_;
}

// - Actions -

void UiManager::refreshTexts()
{
  if (!d_->textsDirty)
    return;
  d_->textsDirty = false;

  //QHash<QString, QVariant> texts;
  QVariantHash texts;
  foreach (const TextEntry &e, d_->entries)
    if (!d_->trs.contains(e.hash))
      texts[QString::number(e.hash)] = e.text;

  if (!texts.isEmpty()) {
    QString json = QtJson::stringify(texts);
    emit textsChanged(json);
  }
}

void UiManager::clearTranslation()
{
  d_->trs.clear();
  d_->touchTexts();
}

void UiManager::updateTranslationData(const QString &json)
{
  QVariant data = QxtJSON::parse(json);
  if (data.isNull())
    return;
  QVariantMap map = data.toMap();
  if (map.isEmpty())
    return;

  for (auto it = map.constBegin(); it != map.constEnd(); ++it)
    if (qint64 hash = it.key().toLongLong()) {
      QString t = d_->trs[hash] = it.value().toString();
      d_->h_trs.insert(Ui::hashString(t));
    }
}

// - Queries -

//bool UiManager::containsWindow(WId window, Ui::TextRole role) const
//{
  //QMutexLocker locker(&d_->mutex);
//  foreach (const Entry &e, d_->entries)
//    if (e.window == window && (!role || role == e.role))
//      return true;
//  return false;
//}

bool UiManager::containsAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return true;
  return false;
}

bool UiManager::containsText(qint64 hash) const
{ return d_->h_texts.find(hash) != d_->h_texts.end(); }

bool UiManager::containsTranslation(qint64 hash) const
{ return d_->h_trs.find(hash) != d_->h_trs.end(); }

const UiManager::TextEntry &UiManager::findTextEntry(qint64 hash) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.hash == hash)
      return e;
  return NULL_TEXT_ENTRY;
}

const UiManager::TextEntry &UiManager::findTextEntryAtAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return e;
  return NULL_TEXT_ENTRY;
}

QString UiManager::findTextTranslation(qint64 hash) const
{ return d_->trs.value(hash); }
//{
//  QString ret = d_->trs.value(hash);
//  if (!ret.isEmpty())
//    ret = findTextEntry(hash).text + " | " + ret;
//  return ret;
//}

void UiManager::updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash)
{
  Q_ASSERT(hash);
  d_->trs[hash] = tr;
  d_->h_trs.insert(trhash ? trhash : Ui::hashString(tr));
}

void UiManager::updateText(const QString &text, qint64 hash, ulong anchor)
{
  Q_ASSERT(anchor);
  if (!hash)
    hash = Ui::hashString(text);
  foreach (const TextEntry &e, d_->entries)
    if (e.hash == hash && e.anchor == anchor)
      return;

  d_->entries.append(TextEntry(text, hash, anchor));

  if (d_->h_texts.find(hash) == d_->h_texts.end()) {
    d_->h_texts.insert(hash);
    d_->touchTexts();
  }
}

// EOF
