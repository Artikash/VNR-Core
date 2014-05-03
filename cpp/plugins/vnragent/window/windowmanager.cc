// windowmanager.cc
// 2/1/2013 jichi

#include "window/windowmanager.h"
#include "window/windowhash.h"
#include "qtjson/qtjson.h"
#include "QxtCore/QxtJSON"
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <unordered_set>

/** Private class */

namespace { WindowManager::TextEntry NULL_TEXT_ENTRY; }

class WindowManagerPrivate
{
  SK_CLASS(WindowManagerPrivate)
  SK_DISABLE_COPY(WindowManagerPrivate)
  //SK_DECLARE_PUBLIC(WindowManager)
  typedef WindowManager Q;

  enum { RefreshInterval = 200 };
  QTimer *refreshTextsTimer_; // QTimer is not working
public:
  bool textsDirty;
  Q::TextEntryList entries;
  QHash<qint64, QString> trs;   // {hash : tr}

  std::unordered_set<qint64> h_texts; // hashes
  std::unordered_set<qint64> h_trs; // hashes

public:
  explicit WindowManagerPrivate(Q *q) : textsDirty(false)
  {
    refreshTextsTimer_ = new QTimer(q);
    refreshTextsTimer_->setSingleShot(true);
    refreshTextsTimer_->setInterval(RefreshInterval);
    q->connect(refreshTextsTimer_, SIGNAL(timeout()), SLOT(refreshTexts()));
  }

  //~WindowManagerPrivate()
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

WindowManager::WindowManager(QObject *parent) : Base(parent), d_(new D(this)) {}

WindowManager::~WindowManager() { delete d_; }

// - Actions -

void WindowManager::refreshTexts()
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

void WindowManager::clearTranslation()
{
  d_->trs.clear();
  d_->touchTexts();
}

void WindowManager::updateTranslation(const QString &json)
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
      d_->h_trs.insert(Window::hashString(t));
    }
}

// - Queries -

//bool WindowManager::containsWindow(WId window, Window::TextRole role) const
//{
  //QMutexLocker locker(&d_->mutex);
//  foreach (const Entry &e, d_->entries)
//    if (e.window == window && (!role || role == e.role))
//      return true;
//  return false;
//}

bool WindowManager::containsAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return true;
  return false;
}

bool WindowManager::containsText(qint64 hash) const
{ return d_->h_texts.find(hash) != d_->h_texts.end(); }

bool WindowManager::containsTranslation(qint64 hash) const
{ return d_->h_trs.find(hash) != d_->h_trs.end(); }

const WindowManager::TextEntry &WindowManager::findTextEntry(qint64 hash) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.hash == hash)
      return e;
  return NULL_TEXT_ENTRY;
}

const WindowManager::TextEntry &WindowManager::findTextEntryAtAnchor(ulong anchor) const
{
  foreach (const TextEntry &e, d_->entries)
    if (e.anchor == anchor)
      return e;
  return NULL_TEXT_ENTRY;
}

QString WindowManager::findTextTranslation(qint64 hash) const
{ return d_->trs.value(hash); }
//{
//  QString ret = d_->trs.value(hash);
//  if (!ret.isEmpty())
//    ret = findTextEntry(hash).text + " | " + ret;
//  return ret;
//}

void WindowManager::updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash)
{
  Q_ASSERT(hash);
  d_->trs[hash] = tr;
  d_->h_trs.insert(trhash ? trhash : Window::hashString(tr));
}

void WindowManager::updateText(const QString &text, qint64 hash, ulong anchor)
{
  Q_ASSERT(anchor);
  if (!hash)
    hash = Window::hashString(text);
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
