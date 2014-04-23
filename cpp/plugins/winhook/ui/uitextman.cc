// dataman.cc
// 2/1/2013 jichi

#include "main.h"
#include "ui/uitextman.h"
#include "ui/uihash.h"
#include "wintimer/wintimer.h"
#include "sakurakit/skhash.h"
#include "QxtCore/QxtJSON"
#include <QtCore/QHash>
#include <unordered_set>

/** Private class */

namespace { DataManager::TextEntry NULL_TEXT_ENTRY; }

class DataManagerPrivate
{
  SK_CLASS(DataManagerPrivate)
  SK_DISABLE_COPY(DataManagerPrivate)
  SK_DECLARE_PUBLIC(DataManager)

  WinTimer refreshTextsTimer_; // QTimer is not working
  bool textsDirty;
public:
  Q::TextEntryList entries;
  QHash<qint64, QString> trs;   // {hash : tr}

  std::unordered_set<qint64> h_texts; // hashes
  std::unordered_set<qint64> h_trs; // hashes

public:
  explicit DataManagerPrivate(Q *q) : q_(q), textsDirty(false)
  {
    refreshTextsTimer_.setInterval(Main::EventLoopInterval / 2);
    refreshTextsTimer_.setMethod(this, &Self::emitChangedTexts);
    // FIXME: Single shot does not work in this environment.
    // After kill a timer, cannot restart it again orz.
    //refreshTextsTimer_.setSingleShot(true);
  }

  ~DataManagerPrivate()
  {
    if (refreshTextsTimer_.isActive())
      refreshTextsTimer_.stop();
  }

  void emitChangedTexts();

  void touchTexts()
  {
    textsDirty = true;
    refreshTextsTimer_.start();
  }
};

// - RPC Data -

void DataManagerPrivate::emitChangedTexts()
{
  if (!textsDirty)
    return;
  textsDirty = false;

  //QHash<QString, QVariant> texts;
  QVariantHash texts;
  foreach (const Q::TextEntry &e, entries)
    if (!trs.contains(e.hash))
      texts[QString::number(e.hash)] = e.text;

  if (!texts.isEmpty()) {
    QString json = QxtJSON::stringify(texts);
    q_->emit textsChanged(json);
  }
}

/** Public class */

// - Construction -

DataManager::DataManager(QObject *parent) : Base(parent), d_(new D(this)) {}
DataManager::~DataManager() { delete d_; }

// - Actions -

void DataManager::clearTranslation()
{
  d_->trs.clear();
  d_->touchTexts();
}

void DataManager::updateTranslation(const QString &json)
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
      d_->h_trs.insert(My::hashString(t));
    }
}

// - Queries -

//bool DataManager::containsWindow(WId window, My::TextRole role) const
//{
  //QMutexLocker locker(&d_->mutex);
//  foreach (const Entry &e, d_->entries)
//    if (e.window == window && (!role || role == e.role))
//      return true;
//  return false;
//}

bool DataManager::containsAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return true;
  return false;
}

bool DataManager::containsText(qint64 hash) const
{ return d_->h_texts.find(hash) != d_->h_texts.end(); }

bool DataManager::containsTranslation(qint64 hash) const
{ return d_->h_trs.find(hash) != d_->h_trs.end(); }

const DataManager::TextEntry &DataManager::findTextEntry(qint64 hash) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.hash == hash)
      return e;
  return NULL_TEXT_ENTRY;
}

const DataManager::TextEntry &DataManager::findTextEntryAtAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return e;
  return NULL_TEXT_ENTRY;
}

QString DataManager::findTextTranslation(qint64 hash) const
{ return d_->trs.value(hash); }
//{
//  QString ret = d_->trs.value(hash);
//  if (!ret.isEmpty())
//    ret = findTextEntry(hash).text + " | " + ret;
//  return ret;
//}

void DataManager::updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash)
{
  Q_ASSERT(hash);
  d_->trs[hash] = tr;
  d_->h_trs.insert(trhash ? trhash : My::hashString(tr));
}

void DataManager::updateText(const QString &text, qint64 hash, ulong anchor)
{
  Q_ASSERT(anchor);
  if (!hash)
    hash = My::hashString(text);
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
